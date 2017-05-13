// vim:ts=2:sts=2:sw=2:cin:et
// -------------------------------------------------------------------
//
//! \author       Robert Luberda
//! \date         16-11-2011
//! \file         SoloFarePathFactory.cpp
//! \brief
//!
//!  Copyright (C) Sabre 2011
//!
//!          The copyright to the computer program(s) herein
//!          is the property of Sabre.
//!          The program(s) may be used and/or copied only with
//!          the written permission of Sabre or in accordance
//!          with the terms and conditions stipulated in the
//!          agreement/contract under which the program(s)
//!          have been supplied.
//
// -------------------------------------------------------------------
#include "Pricing/Shopping/PQ/SoloFarePathFactory.h"

#include "Common/Assert.h"
#include "Common/FLBVisitor.h"
#include "Common/Logger.h"
#include "Common/ShoppingUtil.h"
#include "Common/ShpqTypes.h"
#include "Common/TSELatencyData.h"
#include "DataModel/ItinIndex.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagnosticUtil.h"
#include "Fares/FareByRuleValidator.h"
#include "Fares/FareValidatorOrchestrator.h"
#include "Pricing/FactoriesConfig.h"
#include "Pricing/FarePathValidator.h"
#include "Pricing/PaxFPFBaseData.h"
#include "Pricing/PricingUtil.h"
#include "Pricing/RuleValidator.h"
#include "Pricing/Shopping/PQ/SoloTrxData.h"
#include "Rules/CategoryRuleItemSet.h"
#include "Rules/RuleControllerWithChancelor.h"
#include "Server/TseServer.h"

#include <algorithm>

#include <tr1/functional>

namespace tse
{
namespace shpq
{
namespace
{
Logger
_logger("atseintl.ShoppingPQ.SoloFarePathFactory");

class SoloFarePathFactoryCreator : public FarePathFactoryCreator
{
public:
  virtual FarePathFactory*
  create(const FactoriesConfig& factoriesConfig, const PricingTrx& trx) const override;
};

FarePathFactory*
SoloFarePathFactoryCreator::create(const FactoriesConfig& factoriesConfig, const PricingTrx& trx)
    const
{
  return &trx.dataHandle().safe_create<SoloFarePathFactory>(factoriesConfig);
}

class FareHasQualifierForRule
{
public:
  FareHasQualifierForRule(const PaxTypeFare& fare) : _fare(fare) {}

  bool operator()(uint32_t cat) const { return _fare.qualifyFltAppRuleDataMap().count(cat) != 0; }

private:
  const PaxTypeFare& _fare;
};

bool
isFltIndependentValidationNeeded(const PaxTypeFare& ptf)
{
  return (!ptf.isFltIndependentValidationFVO() || (ptf.isValid() && ptf.flightBitmap().empty()));
}

/**
 * Our goal is to find any non-stop combination which does not exists in flight matrix
 */
class CheckNonStopBitsVisitor : public FLBVisitor
{
public:
  CheckNonStopBitsVisitor(const ShoppingTrx& trx) : _nsCapableSopVec(trx.legs().size()), _trx(trx)
  {
  }

  bool isNSCapable()
  {
    for (const NSSopVec& legNS : _nsCapableSopVec)
    {
      if (legNS.empty())
        return false;
    }

    // transform into internal sops
    for (size_t leg = 0; leg < _nsCapableSopVec.size(); ++leg)
    {
      NSSopVec& legNS = _nsCapableSopVec[leg];
      std::transform(legNS.begin(),
                     legNS.end(),
                     legNS.begin(),
                     std::tr1::bind(&ShoppingUtil::findInternalSopId,
                                    std::tr1::cref(_trx),
                                    leg,
                                    std::tr1::placeholders::_1));
    }

    std::vector<int> sopIdxVec(_trx.legs().size());
    bool result = recursivelyFindNotExistingComb(0, sopIdxVec);

    return result;
  }

  virtual WhatNext visit(PaxTypeFare& ptf,
                         uint16_t legIndex,
                         size_t flbIdx,
                         uint32_t carrierKey,
                         const SOPUsages& sopUsages) override
  {
    const SOPUsage& flbSopUsage = sopUsages[flbIdx];

    if (!flbSopUsage.applicable_)
      return CONTINUE;

    bool isNonStopSOP = (ShoppingUtil::getTravelSegCountForExternalSopID(
                             _trx, legIndex, flbSopUsage.origSopId_) == 1);
    if (!isNonStopSOP)
    {
      bool isNSCapableLeg = !_nsCapableSopVec[legIndex].empty();
      return (isNSCapableLeg ? NEXT_PTF : STOP);
    }

    if (ptf.flightBitmap().empty())
    {
      bool isNSCapablePTF = isFltIndependentValidationNeeded(ptf);
      if (!isNSCapablePTF)
      {
        TSE_ASSERT(_nsCapableSopVec[legIndex].empty());
        return STOP;
      }
    }
    else
    {
      char flightBit = ptf.flightBitmap()[flbIdx]._flightBit;
      bool isNSCapableBit = (flightBit == 0 || flightBit == 'S');
      if (!isNSCapableBit)
      {
        return CONTINUE;
      }
    }

    NSSopVec& legNS = _nsCapableSopVec[legIndex];
    if (legNS.empty())
      legNS.reserve(sopUsages.size());

    legNS.push_back(flbSopUsage.origSopId_);

    return CONTINUE;
  }

private:
  using NSSopVec = std::vector<int>;

  bool recursivelyFindNotExistingComb(const size_t legIdx, std::vector<int>& sopVec) const
  {
    TSE_ASSERT(legIdx < _nsCapableSopVec.size());
    const bool isFinalLegIdx = (legIdx + 1 == _nsCapableSopVec.size());

    for (int nextSop : _nsCapableSopVec[legIdx])
    {
      sopVec[legIdx] = nextSop;

      if (isFinalLegIdx)
      {
        bool isAlreadyGenerated = (_trx.flightMatrix().find(sopVec) != _trx.flightMatrix().end());

        if (!isAlreadyGenerated)
          return true;
      }
      else
      {
        if (UNLIKELY(recursivelyFindNotExistingComb(legIdx + 1, sopVec)))
          return true;
      }
    }

    return false;
  }

  std::vector<NSSopVec> _nsCapableSopVec; // per leg
  const ShoppingTrx& _trx;
};

} // anon ns

// static
SoloFarePathFactory*
SoloFarePathFactory::createFarePathFactory(SoloTrxData& soloTrxData,
                                           PUFBucketVector& puFactoryBucketVector,
                                           SoloFarePathFactory::PUStruct& puStruct)
{
  LOG4CXX_TRACE(_logger, "Entered: createFarePathFactory()");
  std::vector<FarePathFactory*> farePathFactoryBucket;
  ShoppingTrx& shoppingTrx(soloTrxData.getShoppingTrx());

  VALIDATE_OR_THROW(
      shoppingTrx.paxType().size() == 1,
      UNKNOWN_EXCEPTION,
      "Invalid numbers of requested passenger types: " << shoppingTrx.paxType().size());

  VALIDATE_OR_THROW(puFactoryBucketVector.size() == 1,
                    UNKNOWN_EXCEPTION,
                    "Invalid size of puFactoryBucketVecs: " << puFactoryBucketVector.size());

  PricingUnitFactoryBucket* const puFactoryBucket(puFactoryBucketVector.front());

  soloTrxData.getPaxFPFBaseData().trx() = &shoppingTrx;
  soloTrxData.getPaxFPFBaseData().paxType() = shoppingTrx.paxType().front();
  FarePathFactory* fpFactory(
      FarePathFactory::createFarePathFactory(SoloFarePathFactoryCreator(),
                                             puStruct._puPath,
                                             puFactoryBucket,
                                             puStruct._itin,
                                             &soloTrxData.getPaxFPFBaseData()));

  SoloFarePathFactory* result = nullptr;
  if (UNLIKELY(!fpFactory))
  {
    LOG4CXX_ERROR(_logger, "FarePathFactory creation failed");
  }
  else
  {
    result = dynamic_cast<SoloFarePathFactory*>(fpFactory);
    TSE_ASSERT(result != nullptr || !"dynamic_cast failed");
  }
  return result;
}

bool
SoloFarePathFactory::initFarePathFactory(DiagCollector& diag)
{
  TSELatencyData metrics(*_trx, "SOLO INIT PAX FP FACTORY");
  try
  {
    // TODO:  fpf->setEoeCombinabilityEnabled(isEoeCombinabilityEnabled());
    if (FarePathFactory::initFarePathFactory(diag))
    {
      if (LIKELY(FarePathFactory::lowerBoundFPAmount() >= 0))
      {
        // negative -lowerBoundFPAmount means FPF failed to build any
        // FP
        return true;
      }
    }
  }
  catch (...)
  {
    LOG4CXX_ERROR(_logger, "Exception: InitFarePathFactory Failed");
    throw;
  }
  return false;
}

bool
SoloFarePathFactory::isFarePathValid(FPPQItem& fppqItem,
                                     DiagCollector& diag,
                                     std::list<FPPQItem*>&)
{
  const TSELatencyData metrics(*_trx, "SOLO FPF VALIDATION");
  FarePath& farePath = *fppqItem.farePath();
  _validationMsg = nullptr;

  FarePathValidator validator(*_paxFPFBaseData);
  validator.setSettings(FarePathSettings{
      &_allPUF, &_failedPricingUnits, &_eoeFailedFare, _yqyrPreCalc, _externalLowerBoundAmount});
  validator.setEoeCombinabilityEnabled(_eoeCombinabilityEnabled);
  validator.setPuPath(_puPath);
  validator.setItin(_itin);

  if (UNLIKELY(validator.failFareRetailerCodeMismatch(farePath, diag)))
    return false;
  if (UNLIKELY(!isValidSpanishDiscount(farePath)))
    return false;
  if (UNLIKELY(_trx->getRequest()->isBrandedFaresRequest() && !hasBrandParity(farePath, diag)))
    return false;
  if (!checkDirectFlights(farePath))
    return false;
  if (!callbackFVO(farePath, diag))
    return false;
  if (!checkAltDates(farePath, diag))
    return false;
  if (!validator.checkFinalPUBusinessRule(farePath, diag))
    return false;
  if (!validator.checkCombinability(fppqItem, diag))
    return false;
  if (LIKELY(isEoeCombinabilityEnabled()))
    if (UNLIKELY(!FareByRuleValidator::checkSameTariffRule(farePath)))
      return false;
  if (UNLIKELY(!checkCarrierPreference(farePath, diag)))
    return false;

  farepathutils::copyReusablePUToMutablePU(_storage, fppqItem);

  return preValidateFarePath(farePath) && validator.penaltyValidation(farePath);
}

bool
SoloFarePathFactory::callbackFVO(FarePath& farePath, DiagCollector& diag)
{
  ShoppingTrx& shopTrx = dynamic_cast<ShoppingTrx&>(*_trx);
  CxrKeysPerLeg cxrKeysPerLeg;

  if (!ShoppingUtil::collectFPCxrKeysNew(shopTrx, farePath, shopTrx.legs().size(), cxrKeysPerLeg))
    return false;

  for (PricingUnit* pu : farePath.pricingUnit())
  {
    for (FareUsage* fu : pu->fareUsage())
    {
      PaxTypeFare& ptf = *fu->paxTypeFare();
      bool anyCxrKeyValid = false;

      for (uint32_t cxrKey : cxrKeysPerLeg[ptf.fareMarket()->legIndex()])
      {
        ptf.setComponentValidationForCarrier(cxrKey, shopTrx.isAltDates(), shopTrx.mainDuration());

        if (isFltIndependentValidationNeeded(ptf))
        {
          FareValidatorOrchestrator* fvo = static_cast<FareValidatorOrchestrator*>(
              TseServer::getInstance()->service("FARESV_SVC"));
          TSE_ASSERT(fvo);
          fvo->callback(shopTrx, ptf.fareMarket(), &ptf, &cxrKey);
        }

        anyCxrKeyValid = anyCxrKeyValid || (ptf.isValid() && !ptf.flightBitmap().empty());
      }

      if (!anyCxrKeyValid)
        return false;
    }
  }

  return true;
}

bool
SoloFarePathFactory::checkAltDates(FarePath& farePath, DiagCollector& diag)
{
  if (!_trx->isAltDates())
    return true;

  const TSELatencyData metrics(*_trx, "SOLO CHECK ALT DATES");

  ShoppingTrx::AltDatePairs::const_iterator altDateIt = _trx->altDatePairs().begin();
  for (; altDateIt != _trx->altDatePairs().end(); ++altDateIt)
  {
    if (altDateIt->second->numOfSolutionNeeded > 0)
    {
      bool commonValidDate = true;
      std::vector<PricingUnit*>::iterator puIt = farePath.pricingUnit().begin();
      for (; puIt != farePath.pricingUnit().end(); ++puIt)
      {
        std::vector<FareUsage*>::iterator fuIt = (**puIt).fareUsage().begin();
        for (; fuIt != (**puIt).fareUsage().end(); ++fuIt)
        {
          PaxTypeFare& ptf = *(*fuIt)->paxTypeFare();

          if (!ptf.getAltDatePass(altDateIt->first))
          {
            commonValidDate = false;
            break;
          }
        }

        if (!commonValidDate)
          break;
      }

      if (commonValidDate)
        return true;
    }
  }

  return false;
}

bool
SoloFarePathFactory::preValidateFarePath(FarePath& farePath)
{
  ShoppingTrx& shoppingTrx = dynamic_cast<ShoppingTrx&>(*_trx);
  RuleControllerWithChancelor<PricingUnitRuleController> flightIndependentRuleController(
      ShoppingItinFlightIndependentValidation);

  std::vector<PricingUnit*>::iterator puIt = farePath.pricingUnit().begin();
  for (; puIt != farePath.pricingUnit().end(); ++puIt)
  {
    std::vector<uint16_t> categories(flightIndependentRuleController.categorySequence());
    std::vector<FareUsage*>::const_iterator fuIt = (*puIt)->fareUsage().begin();
    for (; fuIt != (*puIt)->fareUsage().end(); ++fuIt)
    {
      const PaxTypeFare& fare = *(*fuIt)->paxTypeFare();
      categories.erase(
          std::remove_if(categories.begin(), categories.end(), FareHasQualifierForRule(fare)),
          categories.end());
    }

    if (UNLIKELY(!categories.empty()))
    {
      flightIndependentRuleController.categorySequence().swap(categories);
      if (!flightIndependentRuleController.validate(shoppingTrx, farePath, **puIt))
        return false;
      flightIndependentRuleController.categorySequence().swap(categories);
    }
  }

  return true;
}

bool
SoloFarePathFactory::checkDirectFlights(FarePath& farePath)
{
  if (!_prevalidateDirectFlights)
    return true;

  TSELatencyData metrics(*_trx, "CHECK DIRECT FLIGHTS");

  ShoppingTrx& shoppingTrx = dynamic_cast<ShoppingTrx&>(*_trx);

  bool result;
  CheckNonStopBitsVisitor checkNSBits(shoppingTrx);

  {
    TSELatencyData metrics(*_trx, "CHECK DIRECT FLIGHTS COLLECT SOPS");
    FLBVisitor::apply(shoppingTrx, farePath, checkNSBits);
  }

  {
    TSELatencyData metrics(*_trx, "CHECK DIRECT FLIGHTS PROCESS SOPS");
    result = checkNSBits.isNSCapable();
  }

  if (!result)
    _validationMsg = "Prevalidate direct flights failed";
  return result;
}

bool
SoloFarePathFactory::isValidSpanishDiscount(FarePath& farePath)
{
  if (UNLIKELY(_trx->getOptions()->getSpanishLargeFamilyDiscountLevel()
                   != SLFUtil::DiscountLevel::NO_DISCOUNT))
  {
    return PricingUtil::validateSpanishDiscountInFP(&farePath);
  }

  return true;
}

} /* namespace shpq */
} /* namespace tse */
