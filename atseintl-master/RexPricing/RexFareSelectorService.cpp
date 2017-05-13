//----------------------------------------------------------------------------
//
// Copyright Sabre 2007
//
//     The copyright to the computer program(s) herein
//     is the property of Sabre.
//     The program(s) may be used and/or copied only with
//     the written permission of Sabre or in accordance
//     with the terms and conditions stipulated in the
//     agreement/contract under which the program(s)
//     have been supplied.
//
//----------------------------------------------------------------------------

#include "RexPricing/RexFareSelectorService.h"

#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "Common/TSELatencyData.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/ProcessTagInfo.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RexExchangeTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/RexShoppingTrx.h"
#include "Diagnostic/Diag689Collector.h"
#include "Diagnostic/Diag690Collector.h"
#include "Diagnostic/DiagManager.h"
#include "Fares/BooleanFlagResetter.h"
#include "RexPricing/CheckTrxError.h"
#include "RexPricing/EnhancedRefundDiscountApplier.h"
#include "RexPricing/PenaltyCalculator.h"
#include "RexPricing/PermutationGenerator.h"
#include "RexPricing/PrepareRexFareRules.h"
#include "RexPricing/RefundPermutationGenerator.h"
#include "Rules/RuleControllerWithChancelor.h"
#include "Server/TseServer.h"

#include <vector>

namespace tse
{
FALLBACK_DECL(fallbackCat31KeepWholeFareSetOnExcFM);
FIXEDFALLBACK_DECL(enhancedRefundDiscountApplierRefactor)

using namespace boost;

static Logger
logger("atseintl.RexFareSelectorService");

static LoadableModuleRegister<Service, RexFareSelectorService>
_("libRexFareSelector.so");

namespace
{
class RoutingValid
{
public:
  void operator()(PaxTypeFare& ptf)
  {
    if (!ptf.isRoutingValid())
      ptf.setRoutingValid(true);
  }
};

typedef RAIIImpl<PaxTypeFare, RoutingValid> RAIISetRoutingValid;

inline bool
isSamePortionOfTravel(const FareMarket* fm1, const FareMarket* fm2)
{
  return fm1->origin()->loc() == fm2->origin()->loc() &&
         fm1->destination()->loc() == fm2->destination()->loc() &&
         fm1->travelSeg().front()->departureDT() ==
         fm2->travelSeg().front()->departureDT();
}

}

RexFareSelectorService::RexFareSelectorService(const std::string& sname, TseServer& tseServer)
  : Service(sname, tseServer), _config(Global::config())
{
}

bool
RexFareSelectorService::initialize(int argc, char* argv[])
{
  LOG4CXX_INFO(logger, "Entering RexFareSelectorService::initialize");

  LOG4CXX_INFO(logger, "Leaving RexFareSelectorService::initialize");
  return true;
}

void
partialFareBreakLimitationSetUp(RexPricingTrx& trx)
{
  for (ProcessTagPermutation* perm : trx.processTagPermutations())
    for (ProcessTagInfo* pti : perm->processTags())
      pti->fareCompInfo()->partialFareBreakLimitationValidation().add(*pti);

  for (FareCompInfo* fci : trx.exchangeItin().front()->fareComponent())
    fci->partialFareBreakLimitationValidation().setUp();
}

bool
RexFareSelectorService::process(RexPricingTrx& trx)
{
  LOG4CXX_INFO(logger, "Entering RexFareSelectorService::process()");
  CheckTrxError checkTrxError(trx);

  if (trx.trxPhase() == RexPricingTrx::MATCH_EXC_RULE_PHASE)
  {
    trx.exchangeItin().front()->determineSegsChangesFor988Match();

    const TSELatencyData metrics(trx, "EXC RULE VALIDATION");
    RuleControllerWithChancelor<PricingUnitRuleController> puRuleController(VolunExcPrevalidation);
    puRuleController.categorySequence().push_back(RuleConst::VOLUNTARY_EXCHANGE_RULE);
    PrepareRexFareRules selector(trx, &puRuleController);
    const bool oldStat = trx.isAnalyzingExcItin();

    trx.setAnalyzingExcItin(true);
    bool result = selector.process();
    trx.setAnalyzingExcItin(oldStat);
    checkTrxError.process(); // would throw exception on error

    processPermutations(trx);
    checkTrxError.checkPermutations();
    partialFareBreakLimitationSetUp(trx);

    return result;
  }
  const TSELatencyData metrics(trx, "EXC FARE MATCHING");
  const bool oldStat = trx.isAnalyzingExcItin();
  trx.setAnalyzingExcItin(true);
  RexFareSelector rfs(trx);
  rfs.process();
  trx.setAnalyzingExcItin(oldStat);

  checkTrxError.process(); // would throw exception on error

  excPlusUpsForNonRefundable(trx);

  LOG4CXX_INFO(logger, "Leaving RexFareSelectorService::process");
  return true;
}

void
RexFareSelectorService::excPlusUpsForNonRefundable(RexPricingTrx& trx) const
{
  if (trx.isPlusUpCalculationNeeded())
  {
    ExcItin& excItin = *trx.exchangeItin()[0];

    if (boost::indeterminate(trx.excTktNonRefundable()))
    {
      RoutingController routingController(trx);
      for (FareCompInfo* fci : excItin.fareComponent())
      {
        for (FareCompInfo::MatchedFare& fare : fci->matchedFares())
        {
          TravelRoute travelRoute;
          TravelRoute travelRouteTktOnly;
          travelRoute.travelRouteTktOnly() = &travelRouteTktOnly;
          RoutingInfos routingInfos;
          RAIISetRoutingValid rsrv(*fare.get());
          routingController.process(*fare.get(), travelRoute, routingInfos);
          if (routingController.isSpecialRouting(*fare.get()) &&
              routingController.needSpecialRouting(*fare.get()))
          {
            std::vector<PaxTypeFare*> fares(1, fare.get());
            routingController.processSpecialRouting(fares, routingInfos, travelRoute);
          }
          //    no diag version for single ptf, that's why
          //    routingController.processRoutingDiagnostic(travelRoute, routingInfos,
          // *fc.fareMarket());
        }
        if (!fallback::fallbackCat31KeepWholeFareSetOnExcFM(&trx))
          fci->loadOtherFares(trx);
      }
    }
    else
    {
      excItin.fareMarket().erase(
          std::remove_if(excItin.fareMarket().begin(),
                         excItin.fareMarket().end(),
                         boost::bind(&Itin::isFareMarketJustForRexPlusUps, &excItin, _1)),
          excItin.fareMarket().end());
    }
  }
}

void
RexFareSelectorService::matchBrandCodesToNewFareMarkets(RexExchangeTrx& trx)
{
  if (trx.getKeepBrandStrategy() != RexExchangeTrx::KEEP_STRATEGY_DISABLED)
  {
    //override exists, dont change strategy
    return;
  }

  trx.setKeepOriginalStrategy(RexExchangeTrx::KEEP_BRAND);
  for (PricingUnit* pu : trx.exchangeItin().front()->farePath().front()->pricingUnit())
  {
    for (FareUsage* fu : pu->fareUsage())
    {
      std::vector<BrandCode> validBrands;
      fu->paxTypeFare()->getValidBrands(trx, validBrands, true);

      if (validBrands.empty())
      {
        trx.setKeepOriginalStrategy(RexExchangeTrx::KEEP_FARE);
        break;
      }

      const FareMarket* excFM = fu->paxTypeFare()->fareMarket();
      for (const FareMarket* newFM : trx.newItin().front()->fareMarket())
      {
        if ((newFM->isFlown() || !newFM->isShopped()) && isSamePortionOfTravel(excFM, newFM))
        {
          for (BrandCode bc : validBrands)
          {
            trx.addNewFMtoBrandCodeSet(newFM, bc);
          }
          break;
        }
      }
    }
  }
}

bool
RexFareSelectorService::process(RexExchangeTrx& trx)
{
  LOG4CXX_INFO(logger, "Entering RexFareSelectorService::process()");
  CheckTrxError checkTrxError(trx);

  if (trx.trxPhase() == RexPricingTrx::MATCH_EXC_RULE_PHASE)
  {
    const TSELatencyData metrics(trx, "EXC RULE VALIDATION");
    RuleControllerWithChancelor<PricingUnitRuleController> puRuleController(VolunExcPrevalidation);
    puRuleController.categorySequence().push_back(RuleConst::VOLUNTARY_EXCHANGE_RULE);
    PrepareRexFareRules selector(trx, &puRuleController);
    const bool oldStat = trx.isAnalyzingExcItin();
    trx.setAnalyzingExcItin(true);

    for (uint16_t itinIndex = 0; itinIndex < trx.newItin().size(); ++itinIndex)
    {
      trx.setItinIndex(itinIndex);
      trx.exchangeItin().front()->determineSegsChangesFor988Match();
      selector.process();
    }
    checkTrxError.multiItinProcess();
    trx.setAnalyzingExcItin(oldStat);
    for (uint16_t itinIndex = 0; itinIndex < trx.newItin().size(); ++itinIndex)
    {
      trx.setItinIndex(itinIndex);
      if (trx.itinStatus() != RexExchangeTrx::REISSUE_RULES_PASS)
        continue;
      processPermutations(trx);
    }

    trx.setItinIndex(0);
    checkTrxError.multiItinCheckPermutations();

    for (uint16_t itinIndex = 0; itinIndex < trx.newItin().size(); ++itinIndex)
    {
      trx.setItinIndex(itinIndex);
      if (trx.itinStatus() != RexExchangeTrx::REISSUE_RULES_PASS)
        continue;
      partialFareBreakLimitationSetUp(trx);
    }

    if(trx.getKeepOriginal())
    {
      matchBrandCodesToNewFareMarkets(trx);
    }

    return true;
  }
  const TSELatencyData metrics(trx, "EXC FARE MATCHING");
  const bool oldStat = trx.isAnalyzingExcItin();
  trx.setAnalyzingExcItin(true);
  RexFareSelector rfs(trx);
  rfs.process();
  trx.setAnalyzingExcItin(oldStat);

  checkTrxError.process(); // would throw exception on error

  LOG4CXX_INFO(logger, "Leaving RexFareSelectorService::process");
  return true;
}

void
RexFareSelectorService::processPermutations(RexPricingTrx& trx) const
{
  PermutationGenerator gen(trx);
  gen.process();
  trx.departureDateValidator().processPermutations();
}

bool
RexFareSelectorService::process(RefundPricingTrx& trx)
{
  LOG4CXX_INFO(logger, "Entering RexFareSelectorService::process()");
  CheckTrxError checkTrxError(trx);

  if (trx.trxPhase() == RexBaseTrx::MATCH_EXC_RULE_PHASE)
  {
    const TSELatencyData metrics(trx, "EXC RULE VALIDATION");

    RuleControllerWithChancelor<PricingUnitRuleController> puRuleController(VolunExcPrevalidation);
    puRuleController.categorySequence().push_back(RuleConst::VOLUNTARY_REFUNDS_RULE);

    PrepareRexFareRules preparator(trx, &puRuleController);
    bool result = preparator.process();

    checkTrxError.process(); // would throw exception on error

    processPermutations(trx);

    return result;
  }

  trx.setAnalyzingExcItin(true);

  const TSELatencyData metrics(trx, "EXC FARE MATCHING");

  RexFareSelector selector(trx);
  selector.process();

  checkTrxError.process(); // would throw exception on error

  LOG4CXX_INFO(logger, "Leaving RexFareSelectorService::process");
  return true;
}

void
RexFareSelectorService::processPermutations(RefundPricingTrx& trx) const
{
  RefundPermutationGenerator gen(trx, logger);
  gen.process(
      trx.permutations(), *trx.exchangeItin().front()->farePath().front(), &trx.refundOptions());

  if (!trx.fullRefund())
    return;

  const RefundDiscountApplier& discountApplier =
      (fallback::fixed::enhancedRefundDiscountApplierRefactor()
           ? *EnhancedRefundDiscountApplier::create(trx.dataHandle(), trx.getExchangePaxType())
           : *RefundDiscountApplier::create(trx.dataHandle(), trx.getExchangePaxType()));

  PenaltyCalculator calc(trx, discountApplier);

  typedef RefundPricingTrx::Permutations::const_iterator It;
  RefundPricingTrx::Permutations& perm = trx.permutations();
  for (It p = perm.begin(); p != perm.end(); ++p)
  {
    calc.calculate(**p);
    trx.setFullRefundWinningPermutation(**p);
  }

  fullRefundDiagnostics(trx);
}

void
RexFareSelectorService::fullRefundDiagnostics(RefundPricingTrx& trx) const
{
  DiagManager man689(trx, Diagnostic689);
  if (man689.isActive())
  {
    Diag689Collector& dc = static_cast<Diag689Collector&>(man689.collector());
    dc.initialize();
    dc.printForFullRefund(trx.exchangeItin().front()->farePath().front()->pricingUnit(),
                          trx.permutations(),
                          trx.exchangeItin().front()->farePath().front()->getTotalNUCAmount());
  }

  DiagManager man690(trx, Diagnostic690);
  if (man690.isActive())
  {
    static_cast<Diag690Collector&>(man690.collector()).printRefundFarePath();
    man690.collector().flushMsg();
  }
}

uint32_t
RexFareSelectorService::getActiveThreads()
{
  return 0; // FareSelector::getActiveThreads();
}

bool
RexFareSelectorService::process(RexShoppingTrx& trx)
{

  LOG4CXX_INFO(logger, "Entered RexFareSelectorService::process");

  trx.setAnalyzingExcItin(true);

  if (trx.trxPhase() == RexPricingTrx::REPRICE_EXCITIN_PHASE)
  {
    const TSELatencyData metrics(trx, "EXC FARE MATCHING");

    RexFareSelector rfs(trx);
    rfs.process();

    CheckTrxError checkTrxError(trx);
    checkTrxError.process(); // would throw exception on error

    LOG4CXX_INFO(logger, "Leaving RexFareSelectorService::process");

    return true;
  }

  const TSELatencyData metrics(trx, "EXC RULE VALIDATION");

  RuleControllerWithChancelor<PricingUnitRuleController> puRuleController(VolunExcPrevalidation);
  puRuleController.categorySequence().push_back(RuleConst::VOLUNTARY_EXCHANGE_RULE);

  PrepareRexFareRules selector(trx, &puRuleController);

  bool result = selector.process();

  CheckTrxError checkTrxError(trx);
  checkTrxError.process();

  LOG4CXX_INFO(logger, "Leaving RexFareSelectorService::process");

  return result;
}
}
