/*---------------------------------------------------------------------------
 *  Copyright Sabre 2015
 *    The copyright to the computer program(s) herein
 *    is the property of Sabre.
 *    The program(s) may be used and/or copied only with
 *    the written permission of Sabre or in accordance
 *    with the terms and conditions stipulated in the
 *    agreement/contract under which the program(s)
 *    have been supplied.
 *-------------------------------------------------------------------------*/

#include "Pricing/SimilarItin/FarePathValidator.h"

#include "Common/FallbackUtil.h"
#include "Common/ValidatingCxrUtil.h"
#include "Common/YQYR/YQYRCalculator.h"
#include "Common/YQYR/YQYRUtils.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "Diagnostic/DiagManager.h"
#include "Pricing/Combinations.h"
#include "Pricing/FactoriesConfig.h"
#include "Pricing/FarePathFactoryFailedPricingUnits.h"
#include "Pricing/FarePathFactoryStorage.h"
#include "Pricing/FarePathUtils.h"
#include "Pricing/FarePathValidator.h"
#include "Pricing/FPPQItemValidator.h"
#include "Pricing/JourneyValidator.h"
#include "Pricing/MIPFamilyLogicUtils.h"
#include "Pricing/PaxFarePathFactoryBase.h"
#include "Pricing/PricingUnitFactoryBucket.h"
#include "Pricing/PricingUtil.h"
#include "Pricing/PUPath.h"
#include "Pricing/SimilarItin/DiagnosticWrapper.h"
#include "Pricing/SimilarItin/SurchargesValidator.h"
#include "Pricing/SimilarItin/VectorSwapper.h"
#include "Pricing/SimilarItin/DataSwappers.h"
#include "Pricing/SimilarItin/ValidationResultsCleaner.h"
#include "Rules/Config.h"

#include <list>
#include <memory>

namespace tse
{
ConfigurableCategories
fpFamilyLogicChildren("FP_FAMILY_LOGIC_CHILDREN");
FALLBACK_DECL(fallbackEnableFamilyLogicAvailiablityValidationInBfa)
FALLBACK_DECL(revalidateVcxForSimilarItins)
FALLBACK_DECL(revalidateVcxForSimilarItins_YQYR)
FALLBACK_DECL(revalidateVcxForSimilarItins_CAT12)
FALLBACK_DECL(validateSurchargesMotherSolution)
FALLBACK_DECL(allValCxrMapYqyrFamilyLogic)
FALLBACK_DECL(gsaSurchargesFix)
FALLBACK_DECL(clearSurchargesForGSARevalidation)
FALLBACK_DECL(setPrimarySectorChildItins)

namespace similaritin
{
namespace
{
void
applySurcharges(PricingTrx& trx, FarePath& farePath)
{
  if (fallback::validateSurchargesMotherSolution(&trx))
    return;

  if (!fallback::gsaSurchargesFix(&trx))
    if (trx.isValidatingCxrGsaApplicable() && !farePath.validatingCarriers().empty())
      farePath.itin()->validatingCarrier() = farePath.validatingCarriers().front();

  DiagManager manager(trx);
  if (UNLIKELY(manager.activate(Diagnostic990).activate(Diagnostic991).isActive()))
  {
    DiagnosticWrapper diagnostic(trx, manager.collector());
    SurchargesValidator<DiagnosticWrapper> validator(trx, diagnostic);
    validator.applySurcharges(farePath);
  }
  else
  {
    NoDiagnostic noDiag;
    SurchargesValidator<NoDiagnostic> validator(trx, noDiag);
    validator.applySurcharges(farePath);
  }
}
}

FarePathValidator::FarePathValidator(PricingTrx& trx, Itin& motherItin)
  : _trx(trx), _motherItin(motherItin)
{
  _storage = _trx.dataHandle().create<FarePathFactoryStorage>();
  _categories = fpFamilyLogicChildren.read();
}

void
FarePathValidator::setConfig(PaxFarePathFactoryBase* paxFPFBase)
{
  _paxFPFBase = paxFPFBase;
}

bool
FarePathValidator::isFarePathValid(FPPQItem& fppqItem,
                                   FarePathSettings& settings,
                                   const SavedFPPQItem::Stage stage,
                                   DiagCollector& diag,
                                   std::list<FPPQItem*>& clonedFPPQItems)
{
  if (fallback::fallbackEnableFamilyLogicAvailiablityValidationInBfa(&_trx) || !_trx.isBRAll())
  {
    if (stage != SavedFPPQItem::Stage::FP_LEVEL)
    {
      applySurcharges(_trx, *fppqItem.farePath());
      return true;
    }

    return isFarePathValidFPLevel(fppqItem, settings, diag, clonedFPPQItems);
  }
  // else
  if (stage != SavedFPPQItem::Stage::FP_LEVEL)
  {
    applySurcharges(_trx, *fppqItem.farePath());
    return isFarePathValidMotherSolution(fppqItem.farePath());
  }
  return isFarePathValidMotherSolution(fppqItem.farePath()) &&
         isFarePathValidFPLevel(fppqItem, settings, diag, clonedFPPQItems);
}

struct GsaRevalidatorGuard
{
  void clearSurcharges(const std::vector<uint16_t>& catSequence, FarePath& farePath)
  {
    ValidationResultsCleaner::clearFpRuleValidationResults(catSequence, farePath);
    for (PricingUnit* pricingUnit : farePath.pricingUnit())
    {
      for (FareUsage* fareUsage : pricingUnit->fareUsage())
      {
        _cleaners.emplace_back(*fareUsage);
        _cleaners.back().needRevalidation(catSequence, farePath);
      }
    }
  }

  std::vector<ValidationResultsCleaner> _cleaners;
};

bool
FarePathValidator::isFarePathValidMotherSolution(FarePath* farePath)
{
  return farepathutils::checkSimilarItinAvailability(
      farePath, PaxTypeUtil::totalNumSeats(_trx), _motherItin);
}

namespace
{
void
prepareBeforeBookingCodeValidation(FarePath& fp, Itin& motherItin)
{
  const auto* itinData = motherItin.getSimilarItinData(*fp.itin());
  TSE_ASSERT(itinData);
  for (auto* const pu : fp.pricingUnit())
  {
    for (auto* const fu : pu->fareUsage())
    {
      FareMarket* fm = fu->paxTypeFare()->fareMarket();
      FareMarket* similarFM = itinData->getSimilarFareMarket(fm);

      fm->availBreaks() = similarFM->availBreaks();
      fm->travelBoundary() = similarFM->travelBoundary();
      fm->primarySector() = similarFM->primarySector();
    }
  }
}
}

bool
FarePathValidator::isFarePathValidFPLevel(FPPQItem& fppqItem,
                                          FarePathSettings& settings,
                                          DiagCollector& diag,
                                          std::list<FPPQItem*>& clonedFPPQItems)
{
  std::list<VectorSwapper<TravelSeg*>> segments;
  for (PricingUnit* pu : fppqItem.farePath()->pricingUnit())
    for (FareUsage* fu : pu->fareUsage())
      segments.emplace_back(fu->travelSeg(), fu->paxTypeFare()->fareMarket()->travelSeg());

  std::unique_ptr<FareMarketDataResetter<FMAvail>> resetter = nullptr;
  if (!fallback::setPrimarySectorChildItins(&_trx))
  {
    resetter = std::make_unique<FareMarketDataResetter<FMAvail>>(*fppqItem.farePath());
    prepareBeforeBookingCodeValidation(*fppqItem.farePath(), _motherItin);
  }
  std::string localRes = "";

  tse::FarePathValidator farePathValidator(*_paxFPFBase);

  farepathutils::copyReusablePUToMutablePU(*_storage, fppqItem);

  bool isValid = farePathValidator.penaltyValidation(*fppqItem.farePath());

  std::vector<FPPQItem*> clonedFpPQ;
  if (fallback::revalidateVcxForSimilarItins(&_trx))
    clonedFpPQ.push_back(&fppqItem);
  else
  {
    if (_trx.isValidatingCxrGsaApplicable())
    {
      clonedFpPQ.reserve(fppqItem.farePath()->validatingCarriers().size() + 1);
      ValidatingCxrUtil::cloneFpPQItemForValCxrs(_trx, fppqItem, clonedFpPQ, *_storage);
    }
    else
      clonedFpPQ.push_back(&fppqItem);
  }

  bool isPrimaryValid = isValid;
  FPPQItemValidator fppqItemValidator(
      *_paxFPFBase, *settings._allPUF, *settings._failedPricingUnits, diag);
  if (fallback::revalidateVcxForSimilarItins_YQYR(&_trx))
    fppqItemValidator.setYqyrCalc(settings._yqyrCalc);

  bool shouldPassCmdPricing = false;
  for (FPPQItem* fpPQI : clonedFpPQ)
  {
    FarePath* farePath = fpPQI->farePath();
    GsaRevalidatorGuard guard;

    if (!fallback::clearSurchargesForGSARevalidation(&_trx))
      guard.clearSurcharges(_categories, *farePath);

    if (!farePath->validatingCarriers().empty())
      farePath->itin()->validatingCarrier() = farePath->validatingCarriers()[0];

    if (!fallback::revalidateVcxForSimilarItins_YQYR(&_trx))
    {
      YQYRCalculator* yqyrCalc = &_trx.dataHandle().safe_create<YQYRCalculator>(
          _trx, *farePath->itin(), farePath, YQYR::YQYRUtils::printDiag(_trx));
      yqyrCalc->process();
      farePath->setYqyrCalculator(yqyrCalc);
      fppqItemValidator.setYqyrCalc(yqyrCalc);
    }

    if (!fallback::revalidateVcxForSimilarItins_CAT12(&_trx))
      ValidationResultsCleaner::clearCat12Surcharges(*farePath);

    bool localValid = isValid;
    fppqItemValidator.validate(fpPQI,
                               localRes,
                               localValid,
                               shouldPassCmdPricing,
                               _paxFPFBase->pricingAxess(),
                               getLowerBoundFPPQItem(),
                               ValidationLevel::SIMILAR_ITIN_FP_LEVEL);

    fpPQI->isValid() = localValid;
    if (fpPQI != &fppqItem)
    {
      // only for clones
      if (localValid)
      {
        farePath->processed() = true;
        clonedFPPQItems.push_back(fpPQI); // keep only if valid
      }
      else
        _storage->releaseFPPQItemDeep(*fpPQI);
    }
    else
      isPrimaryValid = localValid;
  }
  return (isPrimaryValid || !clonedFPPQItems.empty());
}

bool
FarePathValidator::finalValidation(FarePath& farePath)
{
  for (PricingUnit* pu : farePath.pricingUnit())
  {
    for (FareUsage* fu : pu->fareUsage())
    {
      fu->paxTypeFare()->setCategoryProcessed(RuleConst::TICKET_ENDORSMENT_RULE, false);
      fu->paxTypeFare()->setCategorySoftPassed(RuleConst::TICKET_ENDORSMENT_RULE, false);
      PaxTypeFare* baseFare = fu->paxTypeFare()->getBaseFare(RuleConst::FARE_BY_RULE);
      if (baseFare)
      {
        baseFare->setCategoryProcessed(RuleConst::TICKET_ENDORSMENT_RULE, false);
        baseFare->setCategorySoftPassed(RuleConst::TICKET_ENDORSMENT_RULE, false);
      }
    }
  }

  return PricingUtil::finalPricingProcess(_trx, farePath);
}

bool
FarePathValidator::pricedThroughFare(const FarePath& farePath) const
{
  return MIPFamilyLogicUtils::pricedThroughFare(_trx, farePath);
}

void
FarePathValidator::processFarePathClones(FPPQItem*& fppqItem, std::list<FPPQItem*>& clonedFPPQItems)
{
  if (!fallback::revalidateVcxForSimilarItins_YQYR(&_trx) &&
      !fallback::allValCxrMapYqyrFamilyLogic(&_trx))
  {
    auto* calculator = fppqItem->farePath()->yqyrCalculator();
    for (auto clone : clonedFPPQItems)
    {
      auto varCxrMapClone = clone->farePath()->yqyrCalculator()->getValCxrMap();
      calculator->addToValCxrMap(varCxrMapClone);
      clone->farePath()->setYqyrCalculator(calculator);
    }
  }
  ValidatingCxrUtil::processFarePathClones(fppqItem, clonedFPPQItems, *_storage, &_trx);
}
} // similaritin namespace
} // tse namespace
