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
#include "Pricing/SimilarItin/Revalidator.h"

#include "Common/Assert.h"
#include "Common/FallbackUtil.h"
#include "Common/FareMarketUtil.h"
#include "Common/PaxTypeUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/PricingTrx.h"
#include "Diagnostic/Diag990Collector.h"
#include "Fares/RoutingController.h"
#include "Pricing/Combinations.h"
#include "Pricing/FPPQItem.h"
#include "Pricing/FarePathUtils.h"
#include "Pricing/PricingUtil.h"
#include "Pricing/PUPath.h"
#include "Pricing/SimilarItin/Context.h"
#include "Pricing/SimilarItin/DiagnosticWrapper.h"
#include "Pricing/SimilarItin/SurchargesValidator.h"
#include "Pricing/SimilarItin/ValidationResultsCleaner.h"
#include "Pricing/SimilarItin/VectorSwapper.h"
#include "Rules/Config.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtil.h"
#include "Util/BranchPrediction.h"

#include <vector>

namespace tse
{
FALLBACK_DECL(fallbackRoutingForChildren)
FALLBACK_DECL(fallback_FLE_combinability106)
FALLBACK_DECL(fallback_FLE_combinabilityAll)
FALLBACK_DECL(fallbackFixForFlexFaresMultiPax)
FALLBACK_DECL(revalidateVcxForSimilarItins_CAT12)
FALLBACK_DECL(validateSurchargesMotherSolution)

namespace similaritin
{
namespace
{
ConfigurableCategories
fpFamilyLogicChildren("FP_FAMILY_LOGIC_CHILDREN");

std::vector<PUPath*>
getPUPaths(const PricingTrx& trx, const std::vector<FPPQItem*>& gfp)
{
  std::vector<PUPath*> ret;
  if (!fallback::fallbackFixForFlexFaresMultiPax(&trx))
  {
    for (FPPQItem* fppq : gfp)
      ret.push_back(fppq->puPath());

    return ret;
  }
  else
  {
    if (!trx.isFlexFare())
    {
      for (FPPQItem* fppq : gfp)
        ret.push_back(fppq->puPath());

      return ret;
    }
  }
  // safety checks
  if (UNLIKELY(gfp.empty() || !gfp.front()))
    return std::vector<PUPath*>();

  return std::vector<PUPath*>(1, gfp.front()->puPath());
}
}

template <typename D>
Revalidator<D>::RevalidatorConfig::RevalidatorConfig(const PricingTrx& trx)
  : routing(!fallback::fallbackRoutingForChildren(&trx)),
    combinability106(!fallback::fallback_FLE_combinability106(&trx)),
    combinabilityAll(!fallback::fallback_FLE_combinabilityAll(&trx))
{
  categories = fpFamilyLogicChildren.read();
}

template <typename D>
Revalidator<D>::Revalidator(const Context& context, D& diagnostic)
  : _diagnostic(diagnostic), _context(context), _config(_context.trx)
{
}

template <typename D>
bool
Revalidator<D>::validate(const std::vector<FarePath*>& farePathVec,
                         const std::vector<FPPQItem*>& gfp,
                         Itin& est)
{
  uint16_t failedCategory(0);
  if (!validateRule(farePathVec, failedCategory))
  {
    _diagnostic.itinNotApplicableRule(est, failedCategory);
    return false;
  }

  if (!validateRouting(farePathVec))
  {
    _diagnostic.itinNotApplicable(est, Diag990Collector::Routing);
    return false;
  }

  if (!validateCombinability(farePathVec, gfp, est))
  {
    _diagnostic.itinNotApplicable(est, Diag990Collector::Combinability);
    return false;
  }

  if (fallback::revalidateVcxForSimilarItins_CAT12(&_context.trx))
  {
    SurchargesValidator<D> validator(_context.trx, _diagnostic);
    validator.applySurcharges(farePathVec);
  }

  if (!fallback::validateSurchargesMotherSolution(&_context.trx))
    for (auto* farePath : farePathVec)
      PricingUtil::finalPricingProcess(_context.trx, *farePath);

  return true;
}

template <typename D>
bool
Revalidator<D>::validateRule(const std::vector<FarePath*>& farePathVec, uint16_t& category) const
{
  PURuleController ruleController(FPRuleValidation, _config.categories, &_context.trx);
  ruleController.setReuseResult(false);

  for (FarePath* farePath : farePathVec)
  {
    ValidationResultsCleaner::clearFpRuleValidationResults(_config.categories, *farePath);

    for (PricingUnit* pricingUnit : farePath->pricingUnit())
    {
      for (FareUsage* fareUsage : pricingUnit->fareUsage())
      {
        if (!validateFarePathRule(farePath, pricingUnit, fareUsage, ruleController))
        {
          category = ruleController.getProcessedCategory();
          return false;
        }
      }
    }
  }
  return true;
}

template <typename D>
bool
Revalidator<D>::validateFarePathRule(FarePath* farePath,
                                     PricingUnit* pricingUnit,
                                     FareUsage* fareUsage,
                                     PURuleController& ruleController) const
{
  ValidationResultsCleaner cleaner(*fareUsage);
  if (!cleaner.needRevalidation(ruleController.categorySequence(), *farePath))
    return true;

  VectorSwapper<TravelSeg*> travelSegSwapper(fareUsage->travelSeg(),
                                             fareUsage->paxTypeFare()->fareMarket()->travelSeg());
  return ruleController.validate(_context.trx, *farePath, *pricingUnit, *fareUsage);
}
namespace
{
class RoutingRevalidationGuard
{
public:
  explicit RoutingRevalidationGuard(FareUsage* fareUsage)
    : _paxTypeFare(*fareUsage->paxTypeFare()),
      _mileageInfo(_paxTypeFare.mileageInfo()),
      _mileageSurPctg(_paxTypeFare.mileageSurchargePctg()),
      _surchAmt(_paxTypeFare.mileageSurchargeAmt()),
      _surchargeExceptionApplies(_paxTypeFare.surchargeExceptionApplies()),
      _isRoutingProcessed(_paxTypeFare.isRoutingProcessed()),
      _isRoutingValid(_paxTypeFare.isRoutingValid()),
      _vectorSwapper(fareUsage->travelSeg(), _paxTypeFare.fareMarket()->travelSeg())
  {
    _paxTypeFare.setRoutingProcessed(false);
  }

  ~RoutingRevalidationGuard()
  {
    _paxTypeFare.setRoutingProcessed(_isRoutingProcessed);
    _paxTypeFare.setRoutingValid(_isRoutingValid);
    _paxTypeFare.mileageInfo() = _mileageInfo;
    _paxTypeFare.mileageSurchargePctg() = _mileageSurPctg;
    _paxTypeFare.mileageSurchargeAmt() = _surchAmt;
    _paxTypeFare.surchargeExceptionApplies() = _surchargeExceptionApplies;
  }

private:
  PaxTypeFare& _paxTypeFare;
  MileageInfo* _mileageInfo;
  const uint16_t _mileageSurPctg;
  const MoneyAmount _surchAmt;
  const bool _surchargeExceptionApplies;
  const bool _isRoutingProcessed;
  const bool _isRoutingValid;
  VectorSwapper<TravelSeg*> _vectorSwapper;
};
}

template <typename D>
bool
Revalidator<D>::validateRouting(const std::vector<FarePath*>& farePathVec) const
{
  if (!_config.routing)
    return true;

  PricingTrx& trx = _context.trx;
  RoutingController routingController(trx);
  for (FarePath* farePath : farePathVec)
  {
    for (PricingUnit* pricingUnit : farePath->pricingUnit())
    {
      for (FareUsage* fareUsage : pricingUnit->fareUsage())
      {
        PaxTypeFare& paxTypeFare = *fareUsage->paxTypeFare();

        if (routingController.isSpecialRouting(paxTypeFare))
          continue;

        RoutingRevalidationGuard guard(fareUsage);
        TravelRoute travelRoute;
        TravelRoute travelRouteTktOnly;
        travelRoute.travelRouteTktOnly() = &travelRouteTktOnly;
        RoutingInfos routingInfos;

        routingController.process(paxTypeFare, travelRoute, routingInfos);

        routingController.processRoutingDiagnostic(
            travelRoute, routingInfos, *paxTypeFare.fareMarket());
        if (LIKELY(TrxUtil::isFullMapRoutingActivated(trx)) && travelRoute.travelRouteTktOnly())
          routingController.processRoutingDiagnostic(
              *travelRoute.travelRouteTktOnly(), routingInfos, *paxTypeFare.fareMarket());

        if (!paxTypeFare.isRoutingValid())
          return false;
      }
    }
  }

  return true;
}

template <typename D>
bool
Revalidator<D>::validateCombinability(const std::vector<FarePath*>& farePathVec,
                                      const std::vector<FPPQItem*>& gfp,
                                      Itin& est)
{
  if (_config.combinability106)
    return validateCombinability106(farePathVec);

  //do not call for BRALL - found to be to restrictive
  if (_config.combinabilityAll && !_context.trx.isBRAll())
    return validateCombinabilityAll(farePathVec, getPUPaths(_context.trx, gfp), est);

  return true;
}

template <typename D>
bool
Revalidator<D>::validateCombinability106(const std::vector<FarePath*>& farePathVec)
{
  Combinations* combinations(Combinations::getNewInstance(&_context.trx));

  for (FarePath* farePath : farePathVec)
  {
    for (PricingUnit* pricingUnit : farePath->pricingUnit())
    {
      if (!combinations->validate106Only(*pricingUnit, _context.diagnostic))
        return false;
    }
  }
  return true;
}

namespace
{
class FareUsagesSwapper
{
public:
  FareUsagesSwapper(std::vector<FareUsage*>& fareUsages)
  {
    _vectorSwappers.reserve(fareUsages.size());
    for (FareUsage* fareUsage : fareUsages)
    {
      PaxTypeFare& paxTypeFare = *(fareUsage->paxTypeFare());
      _vectorSwappers.emplace_back(fareUsage->travelSeg(), paxTypeFare.fareMarket()->travelSeg());
    }
  }

private:
  std::vector<VectorSwapper<TravelSeg*>> _vectorSwappers;
};
}

template <typename D>
bool
Revalidator<D>::validateCombinabilityPricingUnit(Combinations* combinations,
                                                 const std::vector<FarePath*>& farePathVec,
                                                 Itin& est)
{

  for (FarePath* fp : farePathVec)
  {
    for (PricingUnit* prU : fp->pricingUnit())
    {
      FareUsagesSwapper swapper(prU->fareUsage());
      FareUsage* failedFareUsage, *failedTargetFareUsage;
      if (combinations->process(
              *prU, failedFareUsage, failedTargetFareUsage, _context.diagnostic, &est) !=
          CVR_PASSED)
        return false;
    }
  }
  return true;
}

template <typename D>
bool
Revalidator<D>::validateCombinabilityAll(const std::vector<FarePath*>& farePathVec,
                                         const std::vector<PUPath*>& puPathVec,
                                         Itin& est)
{
  Combinations* combinations(Combinations::getNewInstance(&_context.trx));

  if (!validateCombinabilityPricingUnit(combinations, farePathVec, est))
    return false;

  TSE_ASSERT(puPathVec.size() == farePathVec.size());

  const unsigned int vecSize(puPathVec.size());

  for (unsigned int i(0); i < vecSize; ++i)
  {
    FarePath* fp(farePathVec[i]);
    const PUPath* puPath(puPathVec[i]);

    if (fp->pricingUnit().size() == 1)
      continue;
    farepathutils::copyPUPathEOEInfo(*fp, puPath);
    FareUsage* failedSourceFareUsage, *failedTargetFareUsage;
    if (combinations->process(
            *fp, 0, failedSourceFareUsage, failedTargetFareUsage, _context.diagnostic) !=
        CVR_PASSED)
      return false;
  }

  for (FarePath* fp : farePathVec)
  {
    for (PricingUnit* prU : fp->pricingUnit())
    {
      for (FareUsage* fu : prU->fareUsage())
      {
        if (!fu->rec2Cat10())
        {
          bool isLocationSwapped(false);

          CombinabilityRuleInfo* pCat10(RuleUtil::getCombinabilityRuleInfo(
              _context.trx, *(fu->paxTypeFare()), isLocationSwapped));
          fu->rec2Cat10() = pCat10;
        }
      }
    }
  }
  return true;
}

template <typename D>
bool
Revalidator<D>::validateFbr(const PaxTypeFare& ptf)
{
  PricingTrx& trx = _context.trx;
  const FareMarket& fm = *ptf.fareMarket();
  const FareByRuleItemInfo& fbrItemInfo = ptf.fareByRuleInfo();

  if (fbrItemInfo.fltSegCnt() && fm.travelSeg().size() > fbrItemInfo.fltSegCnt())
    return false;

  const FBRPaxTypeFareRuleData* fbrData = ptf.getFbrRuleData(RuleConst::FARE_BY_RULE);

  if (fbrData && fbrData->isBaseFareAvailBkcMatched())
  {
    const uint16_t numSeatsRequired = PaxTypeUtil::totalNumSeats(trx);

    const auto& bookingCodes = fbrData->baseFareInfoBookingCodes();
    return std::any_of(bookingCodes.cbegin(),
                       bookingCodes.cend(),
                       [&](BookingCode bkc)
                       { return FareMarketUtil::checkAvailability(fm, bkc, numSeatsRequired); });
  }

  return true;
}

template class Revalidator<DiagnosticWrapper>;
template class Revalidator<NoDiagnostic>;
}
}
