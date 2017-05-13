#include "Common/Rec2Selector.h"
#include "Common/Rec2Filter.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/FareByRuleCtrlInfo.h"
#include "DBAccess/FootNoteCtrlInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag225Collector.h"
#include "Diagnostic/Diag202Collector.h"
#include "Rules/RuleUtil.h"

namespace tse
{
namespace Rec2Selector
{
FareByRuleCtrlInfoVec
getFareByRuleCtrlInfo(PricingTrx& trx, FareByRuleApp& fbrApp, FareMarket& fareMarket)
{
  const std::vector<FareByRuleCtrlInfo*>& fbrVec = trx.dataHandle().getAllFareByRuleCtrl(
      fbrApp.vendor(), fbrApp.carrier(), fbrApp.ruleTariff(), fbrApp.ruleNo());

  FareByRuleCtrlInfoVec result = Rec2Filter::CompoundFilter<FareByRuleCtrlInfo>(
                                     trx, fareMarket, fareMarket.travelDate()).matchR2(fbrVec);

  Diag202Collector* dc = Diag202Collector::makeMe(trx, &Diag202Collector::FBR, &fareMarket);
  if (UNLIKELY(dc))
  {
    dc->printR2sMatchDetails<FareByRuleCtrlInfo>(FB_FARE_RULE_RECORD_2,
                                                 trx,
                                                 fbrVec,
                                                 result,
                                                 fareMarket,
                                                 fbrApp.carrier(),
                                                 fareMarket.travelDate());
  }

  return result;
}

FootNoteCtrlInfoVec
getFootNoteCtrl(PricingTrx& trx,
                const PaxTypeFare& paxTypeFare,
                const TariffNumber fareTariff,
                const Footnote footnote,
                const uint16_t categoryNumber,
                const DateTime& travelDate)
{
  const CarrierCode& carrier =
      paxTypeFare.fare()->isIndustry() ? INDUSTRY_CARRIER : paxTypeFare.carrier();

  const std::vector<FootNoteCtrlInfo*>& allFtn = trx.dataHandle().getAllFootNoteCtrl(
      paxTypeFare.vendor(), carrier, fareTariff, footnote, categoryNumber);

  return Rec2Filter::CompoundFilter<FootNoteCtrlInfo>(trx, *paxTypeFare.fareMarket(), travelDate)
      .matchR2(allFtn);
}

FootNoteCtrlInfoVec
getExactFareMatchFootNoteCtrl(PricingTrx& trx,
                              FootNoteCtrlInfoVec& fnCtrlInfoVec,
                              const PaxTypeFare& paxTypeFare)
{
  for (FootNoteCtrlInfoPair& fnPair : fnCtrlInfoVec)
  {
    if (RuleUtil::matchOneWayRoundTrip(fnPair.first->owrt(), paxTypeFare.owrt()) &&
        RuleUtil::matchFareClass(fnPair.first->fareClass().c_str(),
                                 paxTypeFare.fareClass().c_str()) &&
        RuleUtil::matchFareRouteNumber(
            fnPair.first->routingAppl(), fnPair.first->routing(), paxTypeFare.routingNumber()))
    {
      return FootNoteCtrlInfoVec(1, fnPair);
    }
  }

  return FootNoteCtrlInfoVec();
}

GeneralFareRuleInfoVec
getGfr(PricingTrx& trx,
       const PaxTypeFare& paxTypeFare,
       const TariffNumber fareTariff,
       const RuleNumber ruleNumber,
       const uint16_t categoryNumber,
       const DateTime& travelDate)
{
  const CarrierCode& carrier =
      paxTypeFare.fare()->isIndustry() ? INDUSTRY_CARRIER : paxTypeFare.carrier();

  const std::vector<GeneralFareRuleInfo*>& allGfr = trx.dataHandle().getAllGeneralFareRule(
      paxTypeFare.vendor(), carrier, fareTariff, ruleNumber, categoryNumber);

  return Rec2Filter::CompoundFilter<GeneralFareRuleInfo>(trx, *paxTypeFare.fareMarket(), travelDate)
      .matchR2(allGfr);
}
}
}
