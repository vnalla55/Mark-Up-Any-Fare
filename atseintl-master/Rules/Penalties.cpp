#include "Rules/Penalties.h"

#include "Common/CurrencyUtil.h"
#include "Common/DateTime.h"
#include "Common/FallbackUtil.h"
#include "Common/Global.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "Common/TseEnums.h"
#include "Common/TseUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"
#include "DBAccess/LocKey.h"
#include "Diagnostic/Diag316Collector.h"
#include "Diagnostic/DiagManager.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtil.h"
#include "Rules/RuleValidationChancelor.h"
#include "Util/BranchPrediction.h"

#include <exception>

namespace tse
{

FIXEDFALLBACK_DECL(validateAllCat16Records);

static Logger
logger("atseintl.Rules.Penalties");

Penalties::Penalties() {}

static bool
isNonRefundable(const PenaltyInfo& penaltyInfo)
{
  return penaltyInfo.noRefundInd() == Penalties::TICKET_NON_REFUNDABLE ||
         penaltyInfo.noRefundInd() == Penalties::TICKETNRF_AND_RESNOCHANGE;
}

bool
Penalties::nonFlexFareValidationNeeded(PricingTrx& trx) const
{
  return (!trx.isFlexFare() && trx.getOptions()->isNoPenalties());
}

//-------------------------------------------------------------------
//   @method validate
//
//   Description: Performs rule validations on a FareMarket.
//
//   @param PricingTrx           - Pricing transaction
//   @param Itin                 - itinerary
//   @param PaxTypeFare          - reference to Pax Type Fare
//   @param RuleItemInfo         - Record 2 Rule Item Segment Info
//   @param FareMarket           -  Fare Market
//
//   @return Record3ReturnTypes - possible values are:
//                                NOT_PROCESSED = 1
//                                FAIL          = 2
//                                PASS          = 3
//                                SKIP          = 4
//                                STOP          = 5
//
//-------------------------------------------------------------------
Record3ReturnTypes
Penalties::validate(PricingTrx& trx,
                    Itin& itin,
                    const PaxTypeFare& paxTypeFare,
                    const RuleItemInfo* rule,
                    const FareMarket& fareMarket)
{
  LOG4CXX_INFO(logger, " Entered Penalties::validate()");

  LOG4CXX_DEBUG(logger, "Faremarket direction = " << fareMarket.getDirectionAsString());

  const PenaltyInfo* penaltiesRule = dynamic_cast<const PenaltyInfo*>(rule);

  if (!penaltiesRule)
    return SKIP;

  DiagManager diag(trx, Diagnostic316);

  LOG4CXX_DEBUG(logger, "Retrieved Diagnostic");

  // Check for Immediate return conditions
  //
  Record3ReturnTypes retVal = FAIL;

  LOG4CXX_DEBUG(logger, "Ticket Non Refund indicator = " << penaltiesRule->noRefundInd());
  LOG4CXX_DEBUG(logger, "Penalty Amt1 = " << penaltiesRule->penaltyAmt1());
  LOG4CXX_DEBUG(logger, "Penalty Amt2 = " << penaltiesRule->penaltyAmt2());
  LOG4CXX_DEBUG(logger, "Penalty percent = " << penaltiesRule->penaltyPercent());

  retVal = validateUnavailableDataTag(penaltiesRule->unavailTag());

  if (retVal != PASS)
  {
    LOG4CXX_DEBUG(logger, "Data Unavailable failed");

    if (UNLIKELY(diag.isActive()))
    {
      if (retVal == SKIP)
        diag << " PENALTIES: SKIPPED";
      else
        diag << " PENALTIES: FAILED";

      diag << "- UNAVAILABLE DATA TAG : " << penaltiesRule->unavailTag() << "\n";
    }
    return retVal;
  }

  bool fltStopCheck = false;
  TSICode tsiReturn;
  LocKey loc1Return;
  LocKey loc2Return;
  bool origCheck = false;
  bool destCheck = false;
  std::vector<TravelSeg*> travelSegs;
  RuleUtil::TravelSegWrapperVector applTravelSegment;

  if (penaltiesRule->geoTblItemNo() != 0)
  {
    if (!RuleUtil::validateGeoRuleItem(penaltiesRule->geoTblItemNo(),
                                       paxTypeFare.vendor(),
                                       RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT,
                                       false,
                                       false,
                                       false,
                                       trx,
                                       nullptr, // fare path
                                       nullptr,
                                       nullptr, // pricing unit
                                       &fareMarket,
                                       TrxUtil::getTicketingDT(trx),
                                       applTravelSegment, // this will contain the results
                                       origCheck,
                                       destCheck,
                                       fltStopCheck,
                                       tsiReturn,
                                       loc1Return,
                                       loc2Return,
                                       Diagnostic316))
    {
      LOG4CXX_INFO(logger, " Leaving Penalties::validate() - SKIP");

      diag << " PENALTIES: SKIPPED "
           << "- FALSE FROM VALIDATE GEO RULE ITEM\n";

      return SKIP;
    }
  }

  bool nonFlexFareValidation = nonFlexFareValidationNeeded(trx);
  if (nonFlexFareValidation || isValidationNeeded(RuleConst::PENALTIES_RULE, trx))
  {
    Record3ReturnTypes result = validateOptions(penaltiesRule, diag);
    updateStatus(RuleConst::PENALTIES_RULE, result);
    if (nonFlexFareValidation || shouldReturn(RuleConst::PENALTIES_RULE))
      return result;
  }

  // When this indicator is set to YES
  // fares with penalties will be failed
  //

  PaxTypeFare& passengerTypeFare = const_cast<PaxTypeFare&>(paxTypeFare);
  passengerTypeFare.penaltyRestInd() = YES;

  diag << "PENALTIES: SOFTPASSED\n";

  LOG4CXX_INFO(logger, " Leaving Penalties::validate()");

  // Return SOFTPASS in FareComponent scope (this rule should be revalidated in PU scope),
  // because most restrictive Penalty Rule must be saved in FareUsage.

  return SOFTPASS;
}

//-------------------------------------------------------------------
//   @method validate
//
//   Description: Performs rule validations on a PricingUnit.
//
//   @param PricingTrx           - Pricing transaction
//   @param RuleItemInfo         - Record 2 Rule Item Segment Info
//   @param FarePath             - Fare Path
//   @param PricingUnit          - Pricing unit
//   @param FareUsage            - Fare Usage
//
//   @return Record3ReturnTypes  - possible values are:
//                                 NOT_PROCESSED = 1
//                                 FAIL          = 2
//                                 PASS          = 3
//                                 SKIP          = 4
//                                 STOP          = 5
//-------------------------------------------------------------------
Record3ReturnTypes
Penalties::validate(PricingTrx& trx,
                    const RuleItemInfo* rule,
                    const FarePath& farePath,
                    const PricingUnit& pricingUnit,
                    const FareUsage& fareUsage)
{
  return validate(trx, rule, pricingUnit, fareUsage);
}

Record3ReturnTypes
Penalties::validate(PricingTrx& trx,
                    const RuleItemInfo* rule,
                    const PricingUnit& pricingUnit,
                    const FareUsage& fareUsage)
{
  LOG4CXX_INFO(logger, " Entered Penalties::validate() for PricingUnit");
  Record3ReturnTypes retVal = FAIL;

  const PenaltyInfo* penaltiesRule = dynamic_cast<const PenaltyInfo*>(rule);

  if (UNLIKELY(!penaltiesRule))
    return SKIP;

  DiagManager diag(trx, Diagnostic316);

  LOG4CXX_DEBUG(logger, "Retrieved Diagnostic");

  LOG4CXX_DEBUG(logger, "Ticket Non Refund indicator = " << penaltiesRule->noRefundInd());
  LOG4CXX_DEBUG(logger, "Ticket Cancel Refund Appl = " << penaltiesRule->cancelRefundAppl());
  LOG4CXX_DEBUG(logger, "Penalty Amt1 = " << penaltiesRule->penaltyAmt1());
  LOG4CXX_DEBUG(logger, "Penalty Cur1 = " << penaltiesRule->penaltyCur1());
  LOG4CXX_DEBUG(logger, "Penalty Amt2 = " << penaltiesRule->penaltyAmt2());
  LOG4CXX_DEBUG(logger, "Penalty Cur2 = " << penaltiesRule->penaltyCur2());
  LOG4CXX_DEBUG(logger, "Penalty percent = " << penaltiesRule->penaltyPercent());

  // Check for Non refundable fare
     //
   FareUsage& currFareUsage = const_cast<FareUsage&>(fareUsage);

   if (fallback::fixed::validateAllCat16Records())
   {
     if (isNonRefundable(*penaltiesRule))
         currFareUsage.isNonRefundable() = true;
   }
   else
   {
     if (!isNonRefundable(*penaltiesRule) && penaltiesRule->cancelRefundAppl() == APPLIES)
     {
         currFareUsage.isNonRefundable() = false;
         Money penaltyFee = getPenaltyAmount(trx, *currFareUsage.paxTypeFare(), *penaltiesRule,
                                             trx.getRequest()->ticketingAgent()->currencyCodeAgent());

         if (currFareUsage.getNonRefundableAmount().value() < penaltyFee.value())
         {
           currFareUsage.setNonRefundableAmount(penaltyFee);
         }
     }
   }

  // Check for Immediate return conditions
  //
  retVal = validateUnavailableDataTag(penaltiesRule->unavailTag());

  if (retVal != PASS)
  {
    LOG4CXX_DEBUG(logger, "Data Unavailable failed");

    if (UNLIKELY(diag.isActive()))
    {
      if (retVal == SKIP)
        diag << " PENALTIES: SKIPPED";
      else
        diag << " PENALTIES: FAILED";
      diag << "- UNAVAILABLE DATA TAG : " << penaltiesRule->unavailTag() << "\n";
    }

    return retVal;
  }

  bool fltStopCheck = false;
  TSICode tsiReturn;
  LocKey loc1Return;
  LocKey loc2Return;
  bool origCheck = false;
  bool destCheck = false;
  std::vector<TravelSeg*> travelSegs;
  RuleUtil::TravelSegWrapperVector applTravelSegment;

  if (penaltiesRule->geoTblItemNo() != 0)
  {
    if (!RuleUtil::validateGeoRuleItem(penaltiesRule->geoTblItemNo(),
                                       fareUsage.paxTypeFare()->vendor(),
                                       RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT,
                                       false,
                                       false,
                                       false,
                                       trx,
                                       nullptr, // fare path
                                       nullptr,
                                       &pricingUnit, // pricing unit
                                       fareUsage.paxTypeFare()->fareMarket(),
                                       TrxUtil::getTicketingDT(trx),
                                       applTravelSegment, // this will contain the results
                                       origCheck,
                                       destCheck,
                                       fltStopCheck,
                                       tsiReturn,
                                       loc1Return,
                                       loc2Return,
                                       Diagnostic316))
    {
      LOG4CXX_INFO(logger, " Leaving Penalties::validate() - SKIP");

      diag << " PENALTIES: SKIPPED "
           << "- FALSE FROM VALIDATE GEO RULE ITEM\n";

      return SKIP;
    }
  }

  // Check if Request wants to accept fares
  // with penalties
  //
  bool nonFlexFareValidation = nonFlexFareValidationNeeded(trx);
  if (nonFlexFareValidation || isValidationNeeded(RuleConst::PENALTIES_RULE, trx))
  {
    Record3ReturnTypes result = validateOptions(penaltiesRule, diag);
    updateStatus(RuleConst::PENALTIES_RULE, result);
    if (nonFlexFareValidation || shouldReturn(RuleConst::PENALTIES_RULE))
      return result;
  }
  // When this indicator is set to YES
  // fares with penalties will be failed
  //

  currFareUsage.penaltyRestInd() = YES;
  diag << "PENALTIES: PASSED\n";

  // If this is a no change penalty, we set the flag on PaxTypeFare

  if (!currFareUsage.changePenaltyApply())
  {
    if (isThereChangePenalty(*penaltiesRule))
      currFareUsage.changePenaltyApply() = true;
  }

  return PASS;
}

Money
Penalties::getPenaltyAmount(PricingTrx& trx,
                            const PaxTypeFare& ptf,
                            const PenaltyInfo& penaltiesRule,
                            const CurrencyCode& paymentCurrency,
                            const FareUsage* fareUsage,
                            const ConversionType conversionType)
{
  Money penalty(0.0, NUC);

  if (penaltiesRule.penaltyCur2() == ptf.currency())
  {
    penalty = Money(penaltiesRule.penaltyAmt2(),
                    penaltiesRule.penaltyCur2());
  }
  else if (penaltiesRule.penaltyAmt1() > EPSILON)
  {
    penalty = Money(penaltiesRule.penaltyAmt1(),
                    penaltiesRule.penaltyCur1());
  }
  else
  {
    penalty = Money(0.0, paymentCurrency);
  }

  if (penaltiesRule.penaltyPercent() > 0)
  {
    MoneyAmount amountFromPercent = 0.0;

    if (fareUsage)
    {
      amountFromPercent = getPercentagePenaltyFromFU(*fareUsage,
                                                     penaltiesRule.penaltyPercent(),
                                                     paymentCurrency,
                                                     trx, conversionType);
    }
    else
    {
      amountFromPercent = ptf.totalFareAmount() * penaltiesRule.penaltyPercent() / 100.0;
      amountFromPercent = CurrencyUtil::convertMoneyAmount(amountFromPercent, NUC,
                                                           penalty.code(), trx, conversionType);
    }

    penalty.value() = penaltiesRule.penaltyHlInd() ? std::max(penalty.value(), amountFromPercent) :
                                                     std::min(penalty.value(), amountFromPercent);
  }
  MoneyAmount convertedAmount = CurrencyUtil::convertMoneyAmount(penalty.value(), penalty.code(),
                                                                 paymentCurrency, trx,
                                                                 conversionType);
  return Money(convertedAmount, paymentCurrency);
}

MoneyAmount
Penalties::getPercentagePenaltyFromFU(const FareUsage& fu, Percent percent,
                                      const CurrencyCode& paymentCurrency,
                                      PricingTrx& trx,
                                      const ConversionType conversionType)
{
  Money transferMoney(NUC);
  Money stopoverMoney(NUC);
  Money surchareMoney(NUC);
  Money fareMoney(fu.paxTypeFare()->fareAmount(), fu.paxTypeFare()->currency());

  for (auto transferSurcharge : fu.transferSurcharges())
  {
    transferMoney.value() += transferSurcharge.second->unconvertedAmount();
    transferMoney.setCode(transferSurcharge.second->unconvertedCurrencyCode());
  }

  for (auto stopoverSurcharge : fu.stopoverSurcharges())
  {
    stopoverMoney.value() += stopoverSurcharge.second->unconvertedAmount();
    stopoverMoney.setCode(stopoverSurcharge.second->unconvertedCurrencyCode());
  }

  for (auto surcharge : fu.surchargeData())
  {
    surchareMoney.value() += surcharge->amountSelected();
    surchareMoney.setCode(surcharge->currSelected());
  }

  MoneyAmount amt = CurrencyUtil::convertMoneyAmount(transferMoney.value(), transferMoney.code(),
                                                     paymentCurrency, trx, conversionType) +
                    CurrencyUtil::convertMoneyAmount(stopoverMoney.value(), stopoverMoney.code(),
                                                     paymentCurrency, trx, conversionType) +
                    CurrencyUtil::convertMoneyAmount(surchareMoney.value(), surchareMoney.code(),
                                                     paymentCurrency, trx, conversionType) +
                    CurrencyUtil::convertMoneyAmount(fareMoney.value(), fareMoney.code(),
                                                     paymentCurrency, trx, conversionType);

  return (amt * percent) / 100.0;

}

//-------------------------------------------------------------------
//   @method isThereChangePenalty
//
//   Description: Check Penalty Rule to see if there is a change penalty,
//                which will effect NVA/NVB dates
//
//            It will return true if
//                Byte 11 - noRefundInd      - value of N or B
//                                       N = RESERVATIONS_CANNOT_BE_CHANGED
//                                       B = TICKETNRF_AND_RESNOCHANGE
//            Or one or more of these is true and amount or percent > 0
//                Byte 8  - volAppl          - value of X,
//                Byte 14 - penaltyReissue   - value of X,
//                Byte 16 - penaltyNoReissue - value of X
//
//   @param PenaltyInfo          - Penalty rule we are processing
//
//   @return                     - true, there is change penalty
//                                 else false
//
//-------------------------------------------------------------------
bool
Penalties::isThereChangePenalty(const PenaltyInfo& penaltyInfo) const
{
  if ((penaltyInfo.noRefundInd() == RESERVATIONS_CANNOT_BE_CHANGED) ||
      (penaltyInfo.noRefundInd() == TICKETNRF_AND_RESNOCHANGE))
  {
    return true;
  }

  if ((penaltyInfo.volAppl() == APPLIES) || (penaltyInfo.penaltyReissue() == APPLIES) ||
      (penaltyInfo.penaltyNoReissue() == APPLIES))
  {
    if ((penaltyInfo.penaltyAmt1() > 0) || (penaltyInfo.penaltyPercent() > 0))
    {
      return true;
    }
    else
      return false;
  }

  return false;
}

//-------------------------------------------------------------------
//   @method validateOptions
//
//   Description: Check request option and  check if Penalties apply
//
//
//   @param PricingTrx           - Pricing transaction
//
//   @param PenaltyInfo          - Penalty rule we are processing
//
//   @return Record3ReturnTypes  - Fail, if PENALTIES APPLY
//                                 else skip
//
//-------------------------------------------------------------------
Record3ReturnTypes
Penalties::validateOptions(const PenaltyInfo* penaltiesRule, DiagManager& diag) const
{
  if (isNonRefundable(*penaltiesRule) // If the condition is TKT NONREF/NO RES CHG:X or B
      || // no need to check further, it is penalty fare.
      ((penaltiesRule->volAppl() == 'X' ||
        penaltiesRule->involAppl() == 'X') && // If the condition is APPLICABLE TO REROUTE VOL: X
       (penaltiesRule->penaltyAmt1() >
            0 || // or INVOL: X, whether any of the penalty tag is coded or not,
        penaltiesRule->penaltyAmt2() >
            0 || // if the penalty amount/percentage is coded (greater than 0),
        penaltiesRule->penaltyPercent() > 0)) // it is a penalty fare
      ||
      (penaltiesRule->cancelRefundAppl() == 'X' && // Check if the CANCEL/RFND is coded with X;
       (((penaltiesRule->penaltyRefund() ==
              'X' || // if Penalty Tags 1(PenaltyCancel), 3(PenaltyReissue),
          penaltiesRule->penaltyCancel() ==
              'X' || // 5(PenaltyNoReissue), 6(PenaltyRefund) is code AND
          penaltiesRule->penaltyReissue() ==
              'X' || // penalty amount/percentage is coded (greater than 0).
          penaltiesRule->penaltyNoReissue() == 'X') &&
         (penaltiesRule->penaltyAmt1() > 0 || penaltiesRule->penaltyAmt2() > 0 ||
          penaltiesRule->penaltyPercent() > 0)) // If yes, it is a penalty fare, fail the pricing
                                                // when XP/XR
        ||
        (penaltiesRule->penaltyCancel() == ' ' && // are all penalty tags Blank and penalty value
         penaltiesRule->penaltyFail() == ' ' &&
         penaltiesRule->penaltyReissue() == ' ' && penaltiesRule->penaltyExchange() == ' ' &&
         penaltiesRule->penaltyNoReissue() == ' ' && penaltiesRule->penaltyRefund() == ' ' &&
         penaltiesRule->penaltyPta() == ' ' &&
         (penaltiesRule->penaltyAmt1() > 0 || // greater than 0 is coded in either amount1,
          penaltiesRule->penaltyAmt2() > 0 || // amount 2 or percentage?
          penaltiesRule->penaltyPercent() > 0))))) // If yes, it is a penalty fare, fail the pricing
                                                   // when XP/XR.
  {
    diag << "PENALTIES: FAIL "
         << "- PENALTIES APPLY\n";

    return FAIL;
  }
  else
  {
    diag << "PENALTIES: SKIPPED "
         << "- PENALTIES DO NOT APPLY\n";

    return SKIP;
  }
}

} // namespace tse
