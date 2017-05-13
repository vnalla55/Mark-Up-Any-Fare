/*----------------------------------------------------------------------------
 *  Copyright Sabre 2015
 *
 *     The copyright to the computer program(s) herein
 *     is the property of Sabre.
 *     The program(s) may be used and/or copied only with
 *     the written permission of Sabre or in accordance
 *     with the terms and conditions stipulated in the
 *     agreement/contract under which the program(s)
 *     have been supplied.
 *-------------------------------------------------------------------------*/
#pragma once

#include "Common/FallbackUtil.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/SimilarItinData.h"
#include "Pricing/GroupFarePathFactory.h"
#include "Pricing/PaxFarePathFactoryBase.h"
#include "Pricing/SimilarItin/Context.h"
#include "Pricing/SimilarItin/FarePathBuilder.h"
#include "Pricing/SimilarItin/Revalidator.h"
#include "Pricing/SimilarItin/SimilarItinPaxFarePathFactory.h"
#include "Rules/RuleConst.h"

#include <vector>

namespace tse
{
FALLBACK_DECL(gsaSurchargesFix)

namespace similaritin
{
class FarePathValidator;

template <typename D, typename V = FarePathValidator, typename R = Revalidator<D>>
class PriceWithSavedFPPQItems
{
public:
  PriceWithSavedFPPQItems(Context& context, D& diagnostic, V& validator)
    : _builder(context, diagnostic),
      _revalidator(context, diagnostic),
      _context(context),
      _diagnostic(diagnostic),
      _validator(validator)
  {
  }
  bool priceAll(GroupFarePathFactory& gfpf,
                const std::vector<FPPQItem*>* motherGroupFarePath = nullptr)
  {
    bool result = true;
    _diagnostic.processingSavedFPPQItems(_context.motherItin.itinNum());
    for (const SimilarItinData& estData : _context.motherItin.getSimilarItins())
    {
      Itin& est = *estData.itin;
      _diagnostic.processingSimilarItinNo(est.itinNum());
      std::vector<PaxFarePathFactoryBase*> pfpfBucket =
          preparePaxFarePathFactoryBucket(gfpf, motherGroupFarePath, est);
      std::unique_ptr<GroupFarePathFactory> similarItinGFPF =
          getGroupFarePathFactory(gfpf, pfpfBucket);
      if (!priceItinWithCheaperOptions(*similarItinGFPF, est))
        result = false;
    }
    return result;
  }

protected:
  std::tuple<GroupFarePath*, std::vector<FarePath*>>
  getSimilarItinSolution(GroupFarePathFactory& similarItinGFPF, Itin& itin)
  {
    GroupFarePath* similarItinSolution(nullptr);
    while ((similarItinSolution = similarItinGFPF.getGroupFarePath(_context.diagnostic)))
    {
      std::vector<FarePath*> farePathVec;
      for (auto fppqItem : similarItinSolution->groupFPPQItem())
        farePathVec.push_back(fppqItem->farePath());

      if (!fallback::gsaSurchargesFix(&_context.trx))
        if (_context.trx.isValidatingCxrGsaApplicable() &&
            !farePathVec.front()->validatingCarriers().empty())
          itin.validatingCarrier() = farePathVec.front()->validatingCarriers().front();

      if (_revalidator.validate(farePathVec, similarItinSolution->groupFPPQItem(), itin))
      {
        _diagnostic.revalidationPassed();
        return std::make_tuple(similarItinSolution, farePathVec);
      }
      _diagnostic.revalidationFailed();
      similarItinGFPF.buildNextGroupFarePathSet(false, *similarItinSolution, _context.diagnostic);
    }
    return std::make_tuple(nullptr, std::vector<FarePath*>());
  }

  std::unique_ptr<GroupFarePathFactory>
  getGroupFarePathFactory(GroupFarePathFactory& gfpf,
                          const std::vector<PaxFarePathFactoryBase*>& pfpfBucket)
  {
    std::unique_ptr<GroupFarePathFactory> similarItinGFPF(new GroupFarePathFactory(_context.trx));
    similarItinGFPF->setPaxFarePathFactoryBucket(pfpfBucket);
    similarItinGFPF->accompaniedTravel() = gfpf.accompaniedTravel();
    similarItinGFPF->multiPaxShortCktTimeOut() = gfpf.multiPaxShortCktTimeOut();
    return similarItinGFPF;
  }

  bool priceItinWithCheaperOptions(GroupFarePathFactory& similarItinGFPF, Itin& itin)
  {
    if (!similarItinGFPF.initGroupFarePathPQ())
      return false;

    GroupFarePath* similarItinSolution(nullptr);
    std::vector<FarePath*> validFarePaths;

    std::tie(similarItinSolution, validFarePaths) = getSimilarItinSolution(similarItinGFPF, itin);
    if (!similarItinSolution)
      return false;

    for (FarePath* farePath : validFarePaths)
    {
      if (!_validator.finalValidation(*farePath))
      {
        _diagnostic.gfpFailed();
        return false;
      }
    }
    _diagnostic.gfpPassed();
    _diagnostic.itinPopulated(itin, validFarePaths);
    itin.farePath().insert(itin.farePath().end(), validFarePaths.begin(), validFarePaths.end());

    return true;
  }

  void enqueueSolutionsFromMotherItin(PaxFarePathFactoryBase& pfpf,
                                      const std::vector<FPPQItem*>& motherGroupFarePath,
                                      std::vector<SavedFPPQItem>& savedItems)
  {
    auto motherFPPQItemIt = std::find_if(motherGroupFarePath.begin(),
                                         motherGroupFarePath.end(),
                                         [&pfpf](const FPPQItem* item)
                                         { return item->farePath()->paxType() == pfpf.paxType(); });

    if (motherFPPQItemIt != motherGroupFarePath.end())
    {
      savedItems.emplace(savedItems.begin(),
                         savedItems.size(),
                         RuleConst::DUMMY_RULE,
                         (*motherFPPQItemIt)->createDuplicate(_context.trx),
                         *(*motherFPPQItemIt)->getFarePathSettings(),
                         SavedFPPQItem::Stage::MOTHER_SOLUTION);
    }
  }

  std::vector<PaxFarePathFactoryBase*>
  preparePaxFarePathFactoryBucket(GroupFarePathFactory& gfpf,
                                  const std::vector<FPPQItem*>* motherGroupFarePath,
                                  Itin& est)
  {
    std::vector<PaxFarePathFactoryBase*> result;

    PricingTrx& trx = _context.trx;
    for (PaxFarePathFactoryBase* pfpf : gfpf.getPaxFarePathFactoryBucket())
    {
      SimilarItinPaxFarePathFactory<D>* paxFarePathFactory =
          &trx.dataHandle().safe_create<SimilarItinPaxFarePathFactory<D>>(
              pfpf, _validator, _builder, _diagnostic, est, trx);

      auto savedItems = pfpf->getSavedFPPQItems();

      if (motherGroupFarePath != nullptr)
        enqueueSolutionsFromMotherItin(*pfpf, *motherGroupFarePath, savedItems);

      _diagnostic.savedItems(*pfpf->paxType(), savedItems);
      paxFarePathFactory->initQueue(std::move(savedItems));
      paxFarePathFactory->paxType() = pfpf->paxType();
      result.push_back(paxFarePathFactory);
    }

    return result;
  }

private:
  FarePathBuilder<D> _builder;
  R _revalidator;
  Context& _context;
  D& _diagnostic;
  V& _validator;
};
}
}
