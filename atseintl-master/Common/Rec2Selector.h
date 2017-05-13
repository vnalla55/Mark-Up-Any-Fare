#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/DBAForwardDecl.h"
#include <vector>
#include <utility>

namespace tse
{
class PricingTrx;
class FareByRuleApp;
class FareMarket;
class PaxTypeFare;
class DateTime;

namespace Rec2Selector
{
FareByRuleCtrlInfoVec
getFareByRuleCtrlInfo(PricingTrx& trx, FareByRuleApp& fbrApp, FareMarket& fareMarket);

FootNoteCtrlInfoVec
getFootNoteCtrl(PricingTrx& trx,
                const PaxTypeFare& paxTypeFare,
                const TariffNumber fareTariff,
                const Footnote footnote,
                const uint16_t categoryNumber,
                const DateTime& travelDate);

FootNoteCtrlInfoVec
getExactFareMatchFootNoteCtrl(PricingTrx& trx,
                              FootNoteCtrlInfoVec& fnCtrlInfoVec,
                              const PaxTypeFare& paxTypeFare);

GeneralFareRuleInfoVec
getGfr(PricingTrx& trx,
       const PaxTypeFare& paxTypeFare,
       const TariffNumber fareTariff,
       const RuleNumber ruleNumber,
       const uint16_t categoryNumber,
       const DateTime& travelDate);
}
}
