//----------------------------------------------------------------------------
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//----------------------------------------------------------------------------
#pragma once

#include "Common/FallbackUtil.h"
#include "Common/ValidatingCxrUtil.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "Pricing/FPPQItem.h"
#include "Pricing/FactoriesConfig.h"
#include "Pricing/MIPFamilyLogicUtils.h"
#include "Pricing/PaxFarePathFactoryBase.h"
#include "Pricing/PricingUtil.h"
#include "Pricing/SimilarItin/DiagnosticWrapper.h"
#include "Pricing/SimilarItin/FarePathBuilder.h"
#include "Pricing/SimilarItin/FarePathValidator.h"
#include "Pricing/SimilarItin/ValidatingCarrierModule.h"

#include <queue>
#include <vector>

namespace tse
{

FALLBACK_DECL(fallbackGSAChildItinFix2);
FALLBACK_DECL(revalidateVcxForSimilarItins);
FALLBACK_DECL(specifiedCarrierForFamilyLogic);
FALLBACK_DECL(clearGsaClonedFarePathsFamilyLogic);

class FPPQItem;
class FactoriesConfig;
class Itin;
namespace similaritin
{
struct Context;

template <typename D, typename B = FarePathBuilder<D>, typename V = FarePathValidator>
class SimilarItinPaxFarePathFactory : public PaxFarePathFactoryBase
{
  class SavedFPPQItemComparator
  {
  public:
    bool operator()(const SavedFPPQItem& left, const SavedFPPQItem& right) const
    {
      return _comparator(left._fppqItem, right._fppqItem);
    }

  private:
    FPPQItem::GreaterLowToHigh<FPPQItem::GreaterFare> _comparator;
  };
  using SavedFPPQItems =
      std::priority_queue<SavedFPPQItem, std::vector<SavedFPPQItem>, SavedFPPQItemComparator>;

public:
  SimilarItinPaxFarePathFactory(PaxFarePathFactoryBase* baseData,
                                V& validator, B& builder,
                                D& diagnostic,
                                Itin& est,
                                PricingTrx& trx)
    : PaxFarePathFactoryBase(baseData->getFactoriesConfig()),
      _validator(validator),
      _builder(builder),
      _diagnostic(diagnostic),
      _est(est),
      _trx(trx)
  {
    _validator.setConfig(baseData);
    if (!fallback::specifiedCarrierForFamilyLogic(&_trx))
    {
      auto specifiedCarrier = _trx.getRequest()->validatingCarrier();
      if (!specifiedCarrier.empty())
        _est.validatingCarrier() = specifiedCarrier;
    }
  }

  virtual bool initPaxFarePathFactory(DiagCollector&) override { return true; }

  virtual FPPQItem* getFPPQItem(uint32_t idx, DiagCollector& diag) override
  {
    if (idx < _validFPPQItems.size())
      return _validFPPQItems[idx];

    validateNextFPPQItem(diag);

    if (idx < _validFPPQItems.size())
      return _validFPPQItems[idx];

    return nullptr;
  }

  // Get first 0-amount FP for infant that passes CAT13 validation
  virtual FPPQItem*
  getFirstValidZeroAmountFPPQItem(uint32_t, const FPPQItem&, DiagCollector&, uint32_t*) override
  {
    return nullptr;
  }

  virtual FPPQItem* getSameFareBreakFPPQItem(const FPPQItem*, DiagCollector&) override { return nullptr; }

  virtual FPPQItem* getAlreadyBuiltFPPQItem(uint32_t idx) override
  {
    return (idx < _validFPPQItems.size()) ? _validFPPQItems[idx] : nullptr;
  }

  virtual void removeFPF(DiagCollector* diag,
                         const FareMarketPath* fareMarketPathID,
                         const DatePair* datePair = nullptr,
                         bool nonThruPricing = false) override
  {
  }

  virtual bool checkFailedFPIS(FPPQItem*, std::set<DatePair>&, bool) override { return false; }

  virtual void clear() override {}

  void initQueue(std::vector<SavedFPPQItem>&& items)
  {
    SavedFPPQItems(SavedFPPQItemComparator(), std::move(items)).swap(_fppqItems);
    if (!_fppqItems.empty())
      _validator.setLowerBoundFPPQItem(_fppqItems.top()._fppqItem);
  }

private:
 void validateNextFPPQItem(DiagCollector& diag)
  {
    while (!_fppqItems.empty())
    {
      SavedFPPQItem topItem = _fppqItems.top();
      _fppqItems.pop();
      const uint16_t savedItemIndex = topItem._id;
      if (topItem._processed)
      {
        _validFPPQItems.push_back(topItem._fppqItem);
        _diagnostic.returningItem(savedItemIndex);
        return;
      }

      if (!_validator.pricedThroughFare(*topItem._fppqItem->farePath()) &&
          !MIPFamilyLogicUtils::sameCarriers(*topItem._fppqItem->farePath()->itin(), _est))
      {
        _diagnostic.itinNotApplicable(_est, Diag990Collector::SameCarriers);
        continue;
      }

      MIPFamilyLogicUtils::populateSimilarItinData(_trx, _est, *topItem._fppqItem->farePath()->itin());

      FPPQItem* item = _builder.cloneFPPQItem(*topItem._fppqItem, _est);
      if (!item)
        continue;

      bool revalidationRequired = false;

      {
        auto& motherItin = *topItem._fppqItem->farePath()->itin();
        Context context(_trx, motherItin, diag);
        ValidatingCarrierModule validatingCarrierModule(context, _diagnostic.get990Diag());

        if (!validatingCarrierModule.processValidatingCarriers(
                motherItin, *item->farePath(), _est, revalidationRequired))
          continue;
      }
      SavedFPPQItem::Stage stage =
          revalidationRequired ? SavedFPPQItem::Stage::FP_LEVEL : topItem._stage;

      _diagnostic.processingNewFPPQItem(*item, savedItemIndex);

      std::list<FPPQItem*> clonedFPPQItems;
      if (_validator.isFarePathValid(*item, topItem._settings, stage, diag, clonedFPPQItems))
      {
        if (!clonedFPPQItems.empty())
          _validator.processFarePathClones(item, clonedFPPQItems);

        for (auto clone : clonedFPPQItems)
          _validFPPQItems.push_back(clone);

        _diagnostic.savedFPPQItemPassed();
        item->farePath()->processed() = topItem._processed = true;
        if (_fppqItems.empty() ||
            (item->farePath()->getNUCAmountScore() <
             _fppqItems.top()._fppqItem->farePath()->getNUCAmountScore() + EPSILON))
        {
          _validFPPQItems.push_back(item);
          _diagnostic.returningItem(savedItemIndex);
          return;
        }
        topItem._fppqItem = item;
        _fppqItems.push(topItem);
        _diagnostic.pushingBack(savedItemIndex, item->farePath()->getTotalNUCAmount());
      }
      else
        _diagnostic.savedFPPQItemFailed();
    }
  }

  SavedFPPQItems _fppqItems;
  std::vector<FPPQItem*> _validFPPQItems;
  V& _validator;
  B& _builder;
  D& _diagnostic;
  Itin& _est;
  PricingTrx& _trx;
};
}
}
