//-------------------------------------------------------------------
//
//  File:        SalesRestrictionRule.cpp
//  Created:     June 2, 2004
//  Authors:     Vladimir Koliasnikov
//
//  Description:
//
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.    4
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//------------------------------------------------------------------

#include "Common/CarrierUtil.h"
#include "Common/ItinUtil.h"
#include "Common/LocUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Billing.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxType.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RexBaseRequest.h"
#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/Customer.h"
#include "DBAccess/FootNoteCtrlInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Nation.h"
#include "DBAccess/SalesRestriction.h"
#include "Diagnostic/DiagManager.h"
#include "Diagnostic/DiagnosticUtil.h"
#include "Rules/RuleUtil.h"
#include "Rules/SalesRestrictionRule.h"
#include "Rules/UpdaterObserver.h"
#include "Util/BranchPrediction.h"

#include <boost/logic/tribool.hpp>

namespace tse
{

const PseudoCityCode SalesRestrictionRule::OPT_IN_AGENCY = "88D7";

//----------------------------------------------------------------------------
// validate()     Fare component/ Pricing Unit scope
//----------------------------------------------------------------------------

Record3ReturnTypes
SalesRestrictionRule::validate(PricingTrx& trx,
                               Itin& itin,
                               FareUsage* fU,
                               PaxTypeFare& paxTypeFare,
                               const CategoryRuleInfo& cri,
                               const CategoryRuleItemInfo* rule,
                               const SalesRestriction* salesRestriction,
                               bool isQualifiedCategory,
                               bool& isCat15Security,
                               bool skipCat15Security)
{
  bool localeFailed = false;

  return validate(trx,
                  itin,
                  fU,
                  paxTypeFare,
                  cri,
                  rule,
                  salesRestriction,
                  isQualifiedCategory,
                  isCat15Security,
                  skipCat15Security,
                  localeFailed);
}

Record3ReturnTypes
SalesRestrictionRule::validate(PricingTrx& trx,
                               Itin& itin,
                               FareUsage* fU,
                               PaxTypeFare& paxTypeFare,
                               const CombinabilityRuleInfo& cri,
                               const CombinabilityRuleItemInfo* rule,
                               const SalesRestriction* salesRestriction,
                               bool isQualifiedCategory,
                               bool& isCat15Security,
                               bool skipCat15Security)
{
  bool localeFailed = false;

  return validate(trx,
                  itin,
                  fU,
                  paxTypeFare,
                  cri,
                  rule,
                  salesRestriction,
                  isQualifiedCategory,
                  isCat15Security,
                  skipCat15Security,
                  localeFailed);
}

bool
SalesRestrictionRule::refundQualifyingSecurity(const PricingTrx& trx, bool isQualifiedCategory)
    const
{
  return (trx.excTrxType() == PricingTrx::AF_EXC_TRX && isQualifiedCategory &&
          static_cast<const RefundPricingTrx&>(trx).trxPhase() == RexBaseTrx::MATCH_EXC_RULE_PHASE);
}

Record3ReturnTypes
SalesRestrictionRule::determineNotValidReturnCode(const Cat15FailReasons& failReason,
                                                  const PaxTypeFare& paxTypeFare) const
{
  if (failReason == SALES_RESTR_TEXT_ONLY || failReason == SALES_RESTR_DATE_OVERRIDE ||
      (failReason == SALES_RESTR_CRS_OTHER_CARRIER &&
       paxTypeFare.fare()->isCat15GeneralRuleProcess()))
    return SKIP;

  return FAIL;
}

bool
SalesRestrictionRule::isFBRPrivateTariff(const PricingTrx& trx,
                                         const FareUsage* fu,
                                         const PaxTypeFare& ptf) const
{
  return (
      ptf.tcrTariffCat() != RuleConst::PRIVATE_TARIFF &&
      ((fu && fu->cat25Fare() && fu->cat25Fare()->tcrTariffCat() == RuleConst::PRIVATE_TARIFF) ||
       (ptf.cat25Fare() && ptf.cat25Fare()->tcrTariffCat() == RuleConst::PRIVATE_TARIFF)));
}

boost::logic::tribool
SalesRestrictionRule::privateTariffCheck(const PricingTrx& trx,
                                         const PaxTypeFare& paxTypeFare,
                                         const SalesRestriction* salesRestriction,
                                         Diag355& diag355) const
{
  using boost::logic::indeterminate;

  if (paxTypeFare.carrier() == RuleConst::JOINT_CARRIER)
    return true; // any carrier may sell this fare

  if (trx.billing()->partitionID().empty() || trx.billing()->partitionID() == paxTypeFare.carrier())
    return indeterminate;
  if ((salesRestriction->otherCarrier().empty() ||
       !CarrierUtil::carrierExactOrAllianceMatch(
           trx.billing()->partitionID(), salesRestriction->otherCarrier(), trx)) &&
      (!paxTypeFare.fare()->isIndustry() ||
       (trx.billing()->partitionID() != paxTypeFare.fareMarket()->governingCarrier())))
  {
    diag355.printFailMsgPartitionId();
    return false;
  }
  return indeterminate;
}

boost::logic::tribool
SalesRestrictionRule::publicTariffCheck(const PricingTrx& trx,
                                        const PaxTypeFare& paxTypeFare,
                                        const SalesRestriction* salesRestriction,
                                        Diag355& diag355) const
{
  using boost::logic::indeterminate;

  if (!trx.billing()->partitionID().empty() &&
      trx.billing()->partitionID() != paxTypeFare.carrier())
  {
    if (salesRestriction->carrierCrsInd() == RuleConst::MUST_BE_SOLD_VIA_CARRIER)
    {
      if (paxTypeFare.carrier() == RuleConst::JOINT_CARRIER)
        return true; // any carrier may sell this fare

      if ((!paxTypeFare.fare()->isIndustry() &&
           !checkCarrierMatch(
               trx.billing()->partitionID(), salesRestriction->otherCarrier(), trx)) ||
          (paxTypeFare.fare()->isIndustry() && !trx.billing()->partitionID().empty() &&
           trx.billing()->partitionID() != paxTypeFare.fareMarket()->governingCarrier()))
      {
        diag355.printFailMsgRelationalAndCarriers();
        return false;
      } // specified in record 3 Other Carrier field
    } // neither the publishing carrier
    else if (salesRestriction->tvlAgentSaleInd() == RuleConst::MAY_ONLY_BE_SOLD_BY_TA)
    {
      diag355.printFailMsgTvlAgentSaleInd();
      return false;
    }
  }

  return indeterminate;
}

//----------------------------------------------------------------------------
// Validate faregroup
//----------------------------------------------------------------------------
bool
SalesRestrictionRule::validateFareGroup(PricingTrx& trx,
                                        PaxTypeFare& paxTypeFare,
                                        const SalesRestriction* salesRestriction,
                                        std::vector<uint16_t>& actualPaxTypeItem) const
{
  if (actualPaxTypeItem.empty())
  {
    return false; // nothing to do
  }
  std::vector<PaxTypeBucket>& paxTypeCortege = paxTypeFare.fareMarket()->paxTypeCortege();
  // loop for all paxType
  std::vector<uint16_t>::iterator actualPaxTypeItemIt = actualPaxTypeItem.begin();
  std::vector<uint16_t>::iterator actualPaxTypeItemItEnd = actualPaxTypeItem.end();

  for (uint16_t paxTypeNum = 0; actualPaxTypeItemIt != actualPaxTypeItemItEnd;
       ++actualPaxTypeItemIt, ++paxTypeNum)
  {
    if (*actualPaxTypeItemIt == PaxTypeFare::PAXTYPE_FAIL)
    {
      continue;
    }
    if (salesRestriction->locales().empty() == false) // Any Locale item?
    {
      std::vector<PaxType*>::iterator atPaxTypeEnd =
          paxTypeCortege[paxTypeNum].actualPaxType().end();

      std::vector<PaxType*>::iterator atPaxTypeIt =
          paxTypeCortege[paxTypeNum].actualPaxType().begin() + *actualPaxTypeItemIt;
      // loop fare group until pcc matched or paxType change
      PaxTypeCode& paxType = (*atPaxTypeIt)->paxType(); // keep original paxType
      for (; atPaxTypeIt != atPaxTypeEnd; atPaxTypeIt++)
      {
        if (paxType != (*atPaxTypeIt)->paxType())
        {
          *actualPaxTypeItemIt = PaxTypeFare::PAXTYPE_NO_MATCHED; // no matched in this record
          break;
        }
        PosPaxType* posPaxType = dynamic_cast<PosPaxType*>(*atPaxTypeIt);
        if (posPaxType == nullptr)
          return false;

        PseudoCityCode pcc = posPaxType->pcc();
        if (pcc.empty())
        {
          continue; // nothing to do for this paxType
        }
        if (findCorpIdMatch(
                pcc, salesRestriction->locales(), posPaxType, paxTypeFare.matchedCorpID()))
        {
          *actualPaxTypeItemIt = atPaxTypeIt - paxTypeCortege[paxTypeNum].actualPaxType().begin();

          if (!posPaxType->positive()) // matched negative
          {
            *actualPaxTypeItemIt = PaxTypeFare::PAXTYPE_FAIL; // fail it.
          }
          break;
        }
        else
        {
          *actualPaxTypeItemIt = PaxTypeFare::PAXTYPE_NO_MATCHED; // no matched in this record
        }
      }
    }
    else
    {
      *actualPaxTypeItemIt = PaxTypeFare::PAXTYPE_NO_MATCHED; // no matched in this record
    }
  }
  return true;
}

namespace
{
inline bool
isWebAgencyPresent(const LocKey& locKey, const RuleConst::WebAgentLocation agentLoc)
{
  return (locKey.locType() == RuleConst::TRAVEL_AGENCY ||
          locKey.locType() == RuleConst::HOME_TRAVEL_AGENCY) &&
         RuleConst::isWebAgencyPresent(locKey.loc(), agentLoc);
}
}
//----------------------------------------------------------------------------
// isWebFare()
//----------------------------------------------------------------------------
bool
SalesRestrictionRule::isWebFare(const PricingTrx& trx, const SalesRestriction* salesRestriction)
    const
{
  const RuleConst::WebAgentLocation agentLoc =
      hasTvlyLocation(trx) ? RuleConst::TRAVELOCITY : RuleConst::NONTRAVELOCITY;
  auto checkWebAgency = [agentLoc](const Locale* locale)
  {
    return isWebAgencyPresent(locale->loc1(), agentLoc) ||
           isWebAgencyPresent(locale->loc2(), agentLoc);
  };
  return std::any_of(
      salesRestriction->locales().cbegin(), salesRestriction->locales().cend(), checkWebAgency);
}

//----------------------------------------------------------------------------
// checkCountryRestPU()     Final phase to check country restrictions
//                          per directionality
//----------------------------------------------------------------------------
bool
SalesRestrictionRule::checkCountryRestPU(const PricingTrx& trx,
                                         const FareUsage* fareUsage,
                                         const SalesRestriction* salesRestriction) const
{
  //===================================================================
  // check country restriction
  //===================================================================
  if (salesRestriction->countryRest() == RuleConst::BLANK) // Any country restriction ?
    return true;

  const Loc* countryLoc = determineCountryLoc(trx);
  if (countryLoc == nullptr)
  {
    return false;
  }

  // Check country of origin
  const Loc* countryLocCheck;
  const Loc* countryLocCheck2;
  const FareMarket* fm = fareUsage->paxTypeFare()->fareMarket();
  if (salesRestriction->countryRest() == RuleConst::COUNTRY_OF_ORIGIN)
  {
    countryLocCheck = fareUsage->isInbound() ? fm->destination() : fm->origin();

    bool ret = (countryLocCheck->nation() == countryLoc->nation() ||
                checkRussianGroup(*countryLocCheck, *countryLoc));
    fareUsage->paxTypeFare()->track(ret, "C01CNTRY1");
    return ret;
  }
  // Check country of destination
  else if (salesRestriction->countryRest() == RuleConst::COUNTRY_OF_DESTINATION)
  {
    countryLocCheck = fareUsage->isInbound() ? fm->origin() : fm->destination();

    bool ret = (countryLocCheck->nation() == countryLoc->nation() ||
                checkRussianGroup(*countryLocCheck, *countryLoc));
    fareUsage->paxTypeFare()->track(ret, "C01CNTRY2");
    return ret;
  }
  else if (salesRestriction->countryRest() == RuleConst::COUNTRY_OF_ORI_AND_DEST)
  {
    countryLocCheck = fm->origin();
    countryLocCheck2 = fm->destination();

    bool ret = (countryLocCheck->nation() == countryLoc->nation() ||
                countryLocCheck2->nation() == countryLoc->nation() ||
                checkRussianGroup(*countryLocCheck, *countryLoc) ||
                checkRussianGroup(*countryLocCheck2, *countryLoc));
    fareUsage->paxTypeFare()->track(ret, "C01CNTRY3");
    return ret;
  }
  return true;
}

//----------------------------------------------------------------------------
// checkCurrencyRestPU()     Final phase to check country/currency restrictions
//                           per directionality
//----------------------------------------------------------------------------
bool
SalesRestrictionRule::checkCurrencyRestPU(const PricingTrx& trx,
                                          const FareUsage* fareUsage,
                                          const Indicator& currCntryInd,
                                          const CurrencyCode& paymentCurr) const
{
  const FareMarket* fm = fareUsage->paxTypeFare()->fareMarket();

  const NationCode* countryCheck = nullptr;
  const NationCode* countryCheck2 = nullptr;

  //===================================================================
  // check currency restriction
  //===================================================================

  // Check currency of country of origin
  if (currCntryInd == RuleConst::COUNTRY_OF_ORIGIN)
  {
    if (fareUsage->isInbound())
    {
      countryCheck = &(fm->destination()->nation());
    }
    else
    {
      countryCheck = &(fm->origin()->nation());
    }
  }
  else if (currCntryInd == RuleConst::COUNTRY_OF_DESTINATION)
  {
    if (fareUsage->isInbound())
    {
      countryCheck = &(fm->origin()->nation());
    }
    else
    {
      countryCheck = &(fm->destination()->nation());
    }
  }
  else if (currCntryInd == RuleConst::CTR_CURR_OF_ORI_AND_DEST)
  {
    countryCheck = &(fm->origin()->nation());
    countryCheck2 = &(fm->destination()->nation());
  }
  else
  {
    return false;
  }

  CurrencyCode pricingCurrency;
  const Nation* nation = getNation(*countryCheck, fm->travelDate(), trx);
  if (!(nation != nullptr && determinePricingCurrency(*nation, pricingCurrency)))
    return false;

  if (pricingCurrency == paymentCurr)
    return true;

  if (currCntryInd == RuleConst::CTR_CURR_OF_ORI_AND_DEST)
  {
    nation = getNation(*countryCheck2, fm->travelDate(), trx);
    if (!(nation != nullptr && determinePricingCurrency(*nation, pricingCurrency)))
      return false;

    return pricingCurrency == paymentCurr;
  }
  return false;
}

//----------------------------------------------------------------------------
// determinePricingCurrency()
//----------------------------------------------------------------------------
bool
SalesRestrictionRule::determinePricingCurrency(const Nation& nation, CurrencyCode& pricingCurrency)
    const
{
  if (!nation.primeCur().empty())
  {
    pricingCurrency = nation.primeCur();
  }
  else if (!nation.alternateCur().empty())
  {
    pricingCurrency = nation.alternateCur();
  }
  else if (!nation.conversionCur().empty())
  {
    pricingCurrency = nation.conversionCur();
  }
  else
    return false;
  return true;
}

const bool
SalesRestrictionRule::checkPreConditions(Cat15FailReasons& failReason,
                                         const SalesRestriction* salesRestriction,
                                         const bool skipCat15SecurityCheck,
                                         const PricingTrx& trx) const
{
  if (UNLIKELY(salesRestriction->unavailTag() == RuleConst::DATA_UNAVAILABLE))
  {
    failReason = SALES_RESTR_DATA_UNAVAILABLE; // fail this
    return false;
  }

  if (salesRestriction->unavailTag() == RuleConst::TEXT_ONLY)
  {
    failReason = SALES_RESTR_TEXT_ONLY; // skip this cat15
    return false;
  }

  if (!skipCat15SecurityCheck && !isPropperGDS(salesRestriction))
  {
    failReason = SALES_RESTR_CRS_OTHER_CARRIER; // fail this cat15
    return false;
  }

  return true;
}

bool
SalesRestrictionRule::hasTvlyLocation(const PricingTrx& trx) const
{
  if (UNLIKELY((trx.getRequest()->ticketingAgent() == nullptr) ||
                (trx.getRequest()->ticketingAgent()->agentTJR() == nullptr)))
  {
    return false;
  }

  return trx.getRequest()->ticketingAgent()->agentTJR()->tvlyLocation() == YES;
}

//----------------------------------------------------------------------------
// checkCountryRestriction()
//----------------------------------------------------------------------------
bool
SalesRestrictionRule::checkCountryRestriction(PricingTrx& trx,
                                              const FareUsage* fU,
                                              PaxTypeFare& paxTypeFare,
                                              const SalesRestriction* salesRestriction)
{
  if (salesRestriction->countryRest() == RuleConst::BLANK) // Any country restriction
    return true;

  // set warning for WQ trx
  if (trx.noPNRPricing())
    paxTypeFare.warningMap().set(WarningMap::cat15_warning_2, true);

  const Loc* countryLoc = determineCountryLoc(trx);
  if (countryLoc == nullptr)
  {
    return false;
  }

  // The final check of the country restriction will be done in Final phase
  // because this logic is based on the fare selection processing (PU scope).
  //
  // In fare component phase we check the country restriction for any directionality.

  if (fU == nullptr) // Fare Component scope
  {
    const Loc* countryOriginLoc = paxTypeFare.fareMarket()->origin();
    const Loc* countryDestinationLoc = paxTypeFare.fareMarket()->destination();
    if (!(countryOriginLoc->nation() == countryLoc->nation() ||
          countryDestinationLoc->nation() == countryLoc->nation() ||
          checkRussianGroup(*countryOriginLoc, *countryLoc) ||
          checkRussianGroup(*countryDestinationLoc, *countryLoc)))
    {
      return false;
    }
    _softPass = true;
    return true;
  }
  else // Pricing Unit scope
    return checkCountryRestPU(trx, fU, salesRestriction);
}

//----------------------------------------------------------------------------
// checkSalesDate()
//----------------------------------------------------------------------------

bool
SalesRestrictionRule::checkSalesDate(PricingTrx& trx,
                                     PaxTypeFare& paxTypeFare,
                                     const SalesRestriction* salesRestriction) const
{
  DateTime latestSegmentBookingDate;

  const bool isWQTrx = trx.noPNRPricing();
  RuleUtil::getLatestBookingDate(trx, latestSegmentBookingDate, paxTypeFare);

  if (LIKELY(latestSegmentBookingDate.isValid())) // not valid date for the open segment
  {
    if (salesRestriction->earliestResDate().isValid())
    {
      // set warning for WQ trx
      if (UNLIKELY(isWQTrx))
        paxTypeFare.warningMap().set(WarningMap::cat15_warning_2, true);

      if (salesRestriction->earliestResDate().date() > latestSegmentBookingDate.date())
        return false;
      static const std::string C15SALE0("C15SALE0");
      paxTypeFare.track(C15SALE0);
    }
    if (salesRestriction->latestResDate().isValid())
    {
      // set warning for WQ trx
      if (UNLIKELY(isWQTrx))
        paxTypeFare.warningMap().set(WarningMap::cat15_warning_2, true);

      if (salesRestriction->latestResDate().date() < latestSegmentBookingDate.date())
        return false;
      notifyObservers(LAST_DATE_TO_BOOK, salesRestriction->latestResDate());
      static const std::string C15SALE1("C15SALE1");
      paxTypeFare.track(C15SALE1);
    }
  }
  if (salesRestriction->earliestTktDate().isValid())
  {
    // set warning for WQ trx
    if (UNLIKELY(isWQTrx))
      paxTypeFare.warningMap().set(WarningMap::cat15_warning_2, true);

    if (salesRestriction->earliestTktDate().date() > trx.getRequest()->ticketingDT().date())
      return false;
    static const std::string C15SALE2("C15SALE2");
    paxTypeFare.track(C15SALE2);
  }
  if (salesRestriction->latestTktDate().isValid())
  {
    // set warning for WQ trx
    if (UNLIKELY(isWQTrx))
      paxTypeFare.warningMap().set(WarningMap::cat15_warning_2, true);

    if (salesRestriction->latestTktDate().date() < trx.getRequest()->ticketingDT().date())
      return false;
    notifyObservers(LATEST_TKT_DAY, salesRestriction->latestTktDate());
    static const std::string C15SALE3("C15SALE3");
    paxTypeFare.track(C15SALE3);
  }

  return true;
}

//----------------------------------------------------------------------------
// checkFormOfPayment()
//----------------------------------------------------------------------------
bool
SalesRestrictionRule::checkFormOfPayment(const PricingTrx& trx,
                                         PaxTypeFare& paxTypeFare,
                                         const SalesRestriction* salesRestriction,
                                         FareUsage* fareUsage)
{
  if (salesRestriction->fopCashInd() == RuleConst::BLANK &&
      salesRestriction->fopCheckInd() == RuleConst::BLANK &&
      salesRestriction->fopCreditInd() == RuleConst::BLANK &&
      salesRestriction->fopGtrInd() == RuleConst::BLANK)
    return true; // pass for any requested FOP && for any entry type (WP,W',..)

  const bool isWqTrx = trx.noPNRPricing();
  if (UNLIKELY(isWqTrx))
  {
    paxTypeFare.warningMap().set(WarningMap::cat15_warning_2, true);
  }

  if (UNLIKELY(trx.getRequest()->isTicketEntry() || (trx.getRequest()->isFormOfPaymentCash() ||
                                                      trx.getRequest()->isFormOfPaymentCheck() ||
                                                      trx.getRequest()->isFormOfPaymentCard() ||
                                                      trx.getRequest()->isFormOfPaymentGTR())))
    // Ticketing entry or Pricing with input FOP(W' or WP..'FCA)
    return (!((salesRestriction->fopCashInd() != RuleConst::BLANK &&
               trx.getRequest()->isFormOfPaymentCash()) ||
              (salesRestriction->fopCheckInd() != RuleConst::BLANK &&
               trx.getRequest()->isFormOfPaymentCheck()) ||
              (salesRestriction->fopCreditInd() != RuleConst::BLANK &&
               trx.getRequest()->isFormOfPaymentCard()) ||
              (salesRestriction->fopGtrInd() != RuleConst::BLANK &&
               trx.getRequest()->isFormOfPaymentGTR())));

  if (LIKELY(!isWqTrx))
  {
    getTrailerMsg(salesRestriction);
  }
  // for WQ transaction - return true but don't show FOP trailer msg
  // any other entry except W' (ticketing) without input FOP
  // will always return true
  // and will send a trailer MSG if FOP is invalid.

  return true;
}

void
SalesRestrictionRule::getTrailerMsg(const SalesRestriction* salesRestriction)
{
  _forbiddenFop.set(Fare::FOP_CASH, salesRestriction->fopCashInd() != RuleConst::BLANK);
  _forbiddenFop.set(Fare::FOP_CHECK, salesRestriction->fopCheckInd() != RuleConst::BLANK);
  _forbiddenFop.set(Fare::FOP_CREDIT, salesRestriction->fopCreditInd() != RuleConst::BLANK);
  _forbiddenFop.set(Fare::FOP_GTR, salesRestriction->fopGtrInd() != RuleConst::BLANK);
}

void
SalesRestrictionRule::setTrailerMsg(PaxTypeFare& paxTypeFare, FareUsage* fareUsage) const
{
  Fare::FopStatus& forbiddenFop =
      fareUsage ? fareUsage->mutableForbiddenFop() : paxTypeFare.fare()->mutableForbiddenFop();

  if (forbiddenFop.isNull())
    forbiddenFop = _forbiddenFop;
}

//----------------------------------------------------------------------------
// checkCurrencyRestriction()
//----------------------------------------------------------------------------
bool
SalesRestrictionRule::checkCurrencyRestriction(PricingTrx& trx,
                                               const FareUsage* fU,
                                               PaxTypeFare& paxTypeFare,
                                               const SalesRestriction* salesRestriction)
{
  // The relationship between 'curr' and 'currCntryInd' is "OR"
  if (salesRestriction->curr().empty() && // No Currency restriction
      salesRestriction->currCntryInd() == RuleConst::BLANK)
    return true;

  if (trx.noPNRPricing())
  {
    paxTypeFare.warningMap().set(WarningMap::cat15_warning_2, true);
  }

  CurrencyCode paymentCurr;
  if (!trx.getOptions()->currencyOverride().empty())
  {
    paymentCurr = trx.getOptions()->currencyOverride();
  }
  else
  {
    paymentCurr = trx.getRequest()->ticketingAgent()->currencyCodeAgent();
  }

  if (!salesRestriction->curr().empty())
  {
    if (salesRestriction->curr() == paymentCurr)
    {
      return true;
    }
    else if (salesRestriction->currCntryInd() == RuleConst::BLANK)
    {
      return false;
    }
    // try to check "currCntryInd"
  }

  // The final check of the currency restriction will be done in Final phase
  // because this logic is based on the fare selection processing (PU scope).
  //
  // In fare component phase we check the currency restriction for any directionality when:

  if (fU == nullptr) // Fare Component scope
  {
    CurrencyCode pricingCurrency;
    const NationCode* countryOrig = &(paxTypeFare.fareMarket()->origin()->nation());
    const NationCode* countryDest = &(paxTypeFare.fareMarket()->destination()->nation());

    // check currency of origin
    const Nation* nation = getNation(*countryOrig, paxTypeFare.fareMarket()->travelDate(), trx);

    if (!(nation != nullptr && determinePricingCurrency(*nation, pricingCurrency)))
      return false;

    if (pricingCurrency != paymentCurr)
    {
      // check currency of destination
      nation = getNation(*countryDest, paxTypeFare.fareMarket()->travelDate(), trx);
      if (!(nation != nullptr && determinePricingCurrency(*nation, pricingCurrency)))
        return false;

      if (pricingCurrency != paymentCurr)
      {
        return false;
      }
    }

    _softPass = true;
    return true;
  }
  else // Pricing Unit scope
  {
    return checkCurrencyRestPU(trx, fU, salesRestriction->currCntryInd(), paymentCurr);
  }
}

//----------------------------------------------------------------------------
// checkTicketElectronic()
//----------------------------------------------------------------------------
bool
SalesRestrictionRule::checkTicketElectronic(bool reqEtkt, Indicator canEtkt) const
{
  return reqEtkt ? canEtkt != RuleConst::NOT_ALLOWED : canEtkt != RuleConst::REQUIRED;
}

bool
SalesRestrictionRule::checkSaleAndTktLocs(PricingTrx& trx) const
{
  if (!trx.getRequest()->salePointOverride().empty() ||
      !trx.getRequest()->ticketPointOverride().empty())
  {
    Agent* agent = trx.getRequest()->ticketingAgent();
    const Loc* agentLoc = agent->agentLocation();

    if (agentLoc == nullptr)
      return false;

    LocCode salePoint = agent->agentCity();
    LocCode tktPoint = agent->agentCity();
    const Loc* locSale = nullptr;
    const Loc* locTkt = nullptr;

    if (LIKELY(!trx.getRequest()->salePointOverride().empty()))
    {
      salePoint = trx.getRequest()->salePointOverride();
      locSale = getLoc(salePoint, trx);
    }
    else
    {
      locSale = agentLoc;
    }
    if (locSale == nullptr)
      return false;

    if (!trx.getRequest()->ticketPointOverride().empty())
    {
      tktPoint = trx.getRequest()->ticketPointOverride();
      locTkt = getLoc(tktPoint, trx);
    }
    else
    {
      locTkt = agentLoc;
    }

    if (locTkt == nullptr)
      return false;

    LocCode saleCityPoint;
    LocCode tktCityPoint;

    if (!locSale->city().empty())
      saleCityPoint = locSale->city();
    else
      saleCityPoint = locSale->loc();
    if (saleCityPoint == RuleConst::HDQ_CITY)
      saleCityPoint = RuleConst::DFW_CITY;

    if (!locTkt->city().empty())
      tktCityPoint = locTkt->city();
    else
      tktCityPoint = locTkt->loc();
    if (tktCityPoint == RuleConst::HDQ_CITY)
      tktCityPoint = RuleConst::DFW_CITY;

    if (saleCityPoint != tktCityPoint)
    {
      return false;
    }
  }
  return true;
}

//----------------------------------------------------------------------------
// checkSoldTktRestriction()
//----------------------------------------------------------------------------
bool
SalesRestrictionRule::checkSoldTktRestriction(PricingTrx& trx,
                                              Itin& itin,
                                              PaxTypeFare& paxTypeFare,
                                              const SalesRestriction* salesRestriction) const
{
  // Check if there's any type of ticketing/sale transaction restriction
  if ((salesRestriction->tktIssSiti() != RuleConst::BLANK) ||
      (salesRestriction->tktIssSoto() != RuleConst::BLANK) ||
      (salesRestriction->tktIssSito() != RuleConst::BLANK) ||
      (salesRestriction->tktIssSoti() != RuleConst::BLANK))
  {
    bool soldInside = false;
    bool tktInside = false;

    if (UNLIKELY(trx.noPNRPricing()))
    {
      paxTypeFare.warningMap().set(WarningMap::cat15_warning_2, true);
    }

    //------------------------------------------------------------------------
    // Determine what's the Int'l Sales Indicator (SITI/SITO/SOTO/SOTI)
    //------------------------------------------------------------------------

    const NationCode* countrySale;
    const NationCode* countryTkt;
    const NationCode countryOrigin = ItinUtil::originNation(itin);

    const Loc* locSaleTkt;
    const Loc* locOrigin = itin.fareMarket()[0]->origin();

    // Check Point of Sale
    if (!trx.getRequest()->salePointOverride().empty()) // Point of Sale overriden
    {
      locSaleTkt = getLoc(trx.getRequest()->salePointOverride(), trx);
      if (locSaleTkt == nullptr)
      {
        return false;
      }
      countrySale = &(locSaleTkt->nation());
    }
    else
    {
      locSaleTkt = trx.getRequest()->ticketingAgent()->agentLocation();
      countrySale = &(trx.getRequest()->ticketingAgent()->agentLocation()->nation());
    }

    if (*countrySale == countryOrigin)
    {
      soldInside = true;
    }
    else
    {
      if (checkUsTerritoryRule(*locSaleTkt, *locOrigin) ||
          checkRussianGroup(*locSaleTkt, *locOrigin) || checkScandinavia(*locSaleTkt, *locOrigin))
      {
        soldInside = true;
      }
    }

    // Check Point of Tkt
    if (LIKELY(trx.getRequest()->salePointOverride() == trx.getRequest()->ticketPointOverride()))
    {
      tktInside = soldInside;
    }
    else
    {
      if (!trx.getRequest()->ticketPointOverride().empty()) // Point of Tkt overriden
      {
        locSaleTkt = getLoc(trx.getRequest()->ticketPointOverride(), trx);
        if (locSaleTkt == nullptr)
        {
          return false;
        }
        countryTkt = &(locSaleTkt->nation());
      }
      else
      {
        countryTkt = &(trx.getRequest()->ticketingAgent()->agentLocation()->nation());
      }

      if (*countryTkt == countryOrigin)
      {
        tktInside = true;
      }
      else
      {
        if (checkUsTerritoryRule(*locSaleTkt, *locOrigin) ||
            checkRussianGroup(*locSaleTkt, *locOrigin) || checkScandinavia(*locSaleTkt, *locOrigin))
        {
          tktInside = true;
        }
      }
    }

    // Check restriction
    if (soldInside && tktInside) // SITI
    {
      if ((salesRestriction->tktIssSiti() == RuleConst::NOT_ALLOWED) || // Not allowed or
          (salesRestriction->tktIssSiti() == RuleConst::BLANK)) // Not permitted
      {
        return false;
      }
    }
    else if (LIKELY(!soldInside && !tktInside)) // SOTO
    {
      if ((salesRestriction->tktIssSoto() == RuleConst::NOT_ALLOWED) || // Not allowed or
          (salesRestriction->tktIssSoto() == RuleConst::BLANK)) // Not permitted
      {
        return false;
      }
    }
    else if (soldInside && !tktInside) // SITO
    {
      if ((salesRestriction->tktIssSito() == RuleConst::NOT_ALLOWED) || // Not allowed or
          (salesRestriction->tktIssSito() == RuleConst::BLANK)) // Not permitted
      {
        return false;
      }
    }
    else if (!soldInside && tktInside) // SOTI
    {
      if ((salesRestriction->tktIssSoti() == RuleConst::NOT_ALLOWED) || // Not allowed or
          (salesRestriction->tktIssSoti() == RuleConst::BLANK)) // Not permitted
      {
        return false;
      }
    }
  }
  return true;
}

//*******************************
// ValidateCarrier for mip
//*******************************
bool
SalesRestrictionRule::validateCarrier(const PricingTrx& trx,
                                      PaxTypeFare& paxTypeFare,
                                      const SalesRestriction* salesRestriction)
{
  for (const Itin* itin : trx.itin())
  {
    if (UNLIKELY(!paxTypeFare.fareMarket()->hasDuplicates() &&
                  !itin->hasFareMarket(paxTypeFare.fareMarket())))
      continue;
    // at least 1 itin match, return true
    if (trx.isValidatingCxrGsaApplicable() && ruleDataAccess() &&
        !ruleDataAccess()->validatingCxr().empty())
    {
      if (validateCarrier(
              trx, ruleDataAccess()->validatingCxr(), paxTypeFare, salesRestriction, false))
        return true;
    }
    else if (validateCarriers(trx, (*itin), paxTypeFare, salesRestriction, 0, false))
      return true;
  }
  return false;
}
//----------------------------------------------------------------------------
// validateCarrier()
//----------------------------------------------------------------------------
// Validate Carrier Restriction - Validation (Ticket Restriction)
//--------------------------------------------------------------------------
// validatingCarrier is populated in ITIN always now....
bool
SalesRestrictionRule::validateCarrier(const PricingTrx& trx,
                                      const CarrierCode& vcr,
                                      const PaxTypeFare& paxTypeFare,
                                      const SalesRestriction* salesRestriction,
                                      const bool isWQTrx) const
{
  if (UNLIKELY(isWQTrx))
  {
    paxTypeFare.warningMap().set(WarningMap::cat15_warning_1, true);
  }

  if (salesRestriction->validationInd() == RuleConst::VALIDATING_CXR_RESTR_EXCLUDE_PUBLISHING)
    return validateCarrierExcludePublishing(trx, vcr, paxTypeFare, salesRestriction);

  if (checkCarrierMatch(vcr, salesRestriction->otherCarrier(), trx))
    return true;

  if (UNLIKELY(paxTypeFare.carrier() == RuleConst::JOINT_CARRIER))
  {
    const auto& tvlSegVector = paxTypeFare.fareMarket()->travelSeg();
    // Loop through all fare component itinerary items trying to match
    // the flight carrier against the Validating Carrier
    return std::any_of(tvlSegVector.cbegin(),
                       tvlSegVector.cend(),
                       [vcr](const TravelSeg* tvlSeg)
                       { return (tvlSeg->isAir() && tvlSeg->toAirSeg()->carrier() == vcr); });
  }
  return vcr == paxTypeFare.carrier();
}

bool
SalesRestrictionRule::validateCarriers(const PricingTrx& trx,
                                       const Itin& itin,
                                       const PaxTypeFare& paxTypeFare,
                                       const SalesRestriction* salesRestriction,
                                       const FareUsage* fu,
                                       const bool isWQTrx)
{
  if ((fu != 0) || (!trx.isMip()) || !trx.isValidatingCxrGsaApplicable())
  {
    return validateCarrier(trx, itin.validatingCarrier(), paxTypeFare, salesRestriction, isWQTrx);
  }

  std::vector<CarrierCode> valCxrs;
  itin.getValidatingCarriers(trx, valCxrs);

  if (valCxrs.empty() || (valCxrs.size() == 1))
    return validateCarrier(trx, itin.validatingCarrier(), paxTypeFare, salesRestriction, isWQTrx);

  // Multiple VCX case. Return true only if one passes.
  for (CarrierCode vcx : valCxrs)
  {
    if (validateCarrier(trx, vcx, paxTypeFare, salesRestriction, isWQTrx))
    {
      _softPass = true;
      return true;
    }
  }

  return false;
}

bool
SalesRestrictionRule::validateCarrierExcludePublishing(const PricingTrx& trx,
                                                       const CarrierCode& vcr,
                                                       const PaxTypeFare& paxTypeFare,
                                                       const SalesRestriction* salesRestriction)
    const
{
  if (vcr == paxTypeFare.carrier())
    return false;
  if (UNLIKELY(salesRestriction->otherCarrier().empty()))
    return false; // bad data
  return CarrierUtil::carrierExactOrAllianceMatch(vcr, salesRestriction->otherCarrier(), trx);
}

//--------------------------------------------------------------------------
// Validate Carrier Restriction - Segment
//--------------------------------------------------------------------------
// Check if no segment of ticket may be on any carrier except publishing
// carrier or specified carrier (Other Carrier field)
bool
SalesRestrictionRule::validateCarrierRestrictionSegment(const PricingTrx& trx,
                                                        const Itin& itin,
                                                        const PaxTypeFare& paxTypeFare,
                                                        const SalesRestriction* salesRestriction,
                                                        const bool isWQTrx) const
{
  if (isWQTrx)
  {
    paxTypeFare.warningMap().set(WarningMap::cat15_warning_1, true);
  }

  if (paxTypeFare.actualPaxType() != nullptr &&
      paxTypeFare.actualPaxType()->paxType() == RuleConst::MATCH_NEG_PAX_TYPE) //???
  {
    // Check if itinerary has multiple carriers
    bool multipleCarriers = false;
    CarrierCode carrier; //          = " ";

    // Point to start and end of the itinerary

    // Loop through all itinerary items to check the carrier
    for (const TravelSeg* travelSeg : itin.travelSeg())
    {
      const AirSeg* airSeg = travelSeg->toAirSeg();
      if (airSeg && !airSeg->carrier().empty() && airSeg->carrier() != carrier)
      {
        if (carrier.empty()) // 1st time
        {
          carrier = airSeg->carrier();
        }
        else
        {
          multipleCarriers = true;
          break;
        }
      }
    }

    if (!multipleCarriers)
    {
      return true;
    }

    if (salesRestriction->otherCarrier().empty() && // There's no Other Carrier
        paxTypeFare.carrier() != RuleConst::JOINT_CARRIER)
    {
      return !trx.getRequest()->isTicketEntry();
    }
  }
  return checkCarrier(trx, itin, paxTypeFare, salesRestriction);
}

bool
SalesRestrictionRule::checkCarrierMatchForJointCarrier(const PricingTrx& trx,
                                                       const Itin& itin,
                                                       const PaxTypeFare& paxTypeFare,
                                                       const SalesRestriction* salesRestriction)
    const
{
  std::vector<TravelSeg*>::const_iterator tsIt = itin.travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator tsE = itin.travelSeg().end();
  // Loop through all itinerary items to check the carrier
  for (; tsIt != tsE; ++tsIt)
  {
    bool matchFound = false;

    AirSeg* airSeg = dynamic_cast<AirSeg*>(*tsIt);
    if (airSeg != nullptr && !airSeg->carrier().empty())
    {
      // Loop through all fare component segments items
      std::vector<TravelSeg*>::const_iterator tsFareB =
          paxTypeFare.fareMarket()->travelSeg().begin();
      std::vector<TravelSeg*>::const_iterator tsFareE = paxTypeFare.fareMarket()->travelSeg().end();

      for (; tsFareB != tsFareE; ++tsFareB)
      {
        AirSeg* airS = dynamic_cast<AirSeg*>(*tsFareB);
        if (airS != nullptr && !airS->carrier().empty())
        {
          if (airSeg->carrier() == airS->carrier() ||
              checkCarrierMatch(airSeg->carrier(), salesRestriction->otherCarrier(), trx))
          {
            matchFound = true;
            break;
          }
        }
      }

      if (!matchFound)
      {
        if (paxTypeFare.actualPaxType() == nullptr ||
            paxTypeFare.actualPaxType()->paxType() != RuleConst::MATCH_NEG_PAX_TYPE)
        {
          return false;
        }
        return !trx.getRequest()->isTicketEntry(); // ticketing entry
      }
    }
  } // for
  return true;
}

bool
SalesRestrictionRule::checkCarrier(const PricingTrx& trx,
                                   const Itin& itin,
                                   const PaxTypeFare& paxTypeFare,
                                   const SalesRestriction* salesRestriction) const
{
  if (paxTypeFare.carrier() == RuleConst::JOINT_CARRIER)
  {
    return checkCarrierMatchForJointCarrier(trx, itin, paxTypeFare, salesRestriction);
  }
  else // Publishing carrier is not a joint carrier
  {
    return validateCarrierRestriction(
        trx, itin.travelSeg(), paxTypeFare.carrier(), salesRestriction->otherCarrier());
  }
}

bool
SalesRestrictionRule::validateCarrierRestrictionFare(const PricingTrx& trx,
                                                     const PaxTypeFare& paxTypeFare,
                                                     const SalesRestriction* salesRestriction,
                                                     const bool isWQTrx) const
{
  if (isWQTrx)
  {
    paxTypeFare.warningMap().set(WarningMap::cat15_warning_1, true);
  }

  if (paxTypeFare.carrier() == RuleConst::JOINT_CARRIER)
  {
    return true;
  }

  return validateCarrierRestriction(trx,
                                    paxTypeFare.fareMarket()->travelSeg(),
                                    paxTypeFare.carrier(),
                                    salesRestriction->otherCarrier());
}

bool
SalesRestrictionRule::validateCarrierRestriction(const PricingTrx& trx,
                                                 const std::vector<TravelSeg*>& travelSegments,
                                                 const CarrierCode& carrier,
                                                 const CarrierCode& otherCarrier) const
{
  for (const TravelSeg* tvlSeg : travelSegments)
  {
    if (tvlSeg->isAir())
    {
      const CarrierCode segCarrier = tvlSeg->toAirSeg()->carrier();
      if (!segCarrier.empty() && segCarrier != carrier &&
          !checkCarrierMatch(segCarrier, otherCarrier, trx))
        return false;
    }
  }
  return true;
}

//----------------------------------------------------------------------------
// checkCarrierRestriction()
//----------------------------------------------------------------------------
bool
SalesRestrictionRule::checkCarrierRestriction(PricingTrx& trx,
                                              Itin& itin,
                                              PaxTypeFare& paxTypeFare,
                                              const SalesRestriction* salesRestriction,
                                              const FareUsage* fU)
{
  if (salesRestriction->validationInd() == RuleConst::BLANK &&
      salesRestriction->carrierSegInd() == RuleConst::BLANK) // No more restrictions
  {
    return true;
  }

  const bool isWQTrx = trx.noPNRPricing();

  if (LIKELY(salesRestriction->validationInd() != RuleConst::BLANK))
  {
    if (PricingTrx::IS_TRX == trx.getTrxType() && fU == nullptr)
    {
      _softPass = true;
      return true;
    }

    if (PricingTrx::MIP_TRX == trx.getTrxType() && fU == nullptr &&
        (paxTypeFare.fareMarket()->hasDuplicates()))
    {
      // unknown itin context, try to match with any itin
      if (!validateCarrier(trx, paxTypeFare, salesRestriction))
      {
        return false;
      }
      _softPass = true;
    }
    else
    {
      if (trx.isValidatingCxrGsaApplicable() && (PricingTrx::IS_TRX == trx.getTrxType()))
        return validateAllCarriersIS(trx, paxTypeFare, itin, salesRestriction);

      if (ruleDataAccess() && !ruleDataAccess()->validatingCxr().empty())
      {
        if (!validateCarrier(
                trx, ruleDataAccess()->validatingCxr(), paxTypeFare, salesRestriction, false))
          return false;
      }
      else if (!validateCarriers(trx, itin, paxTypeFare, salesRestriction, fU, isWQTrx))
      {
        // Plate, Stock, Either or Both
        return false;
      }
    }
  }

  // Check if no segment at this fare may be on any carrier except publishing
  // carrier or specified carrier (Other Carrier field)
  if (UNLIKELY(salesRestriction->carrierSegInd() == RuleConst::NO_SEGMENT_AT_THIS_FARE))
  {
    if (PricingTrx::MIP_TRX == trx.getTrxType() && fU == nullptr &&
        paxTypeFare.fareMarket()->hasDuplicates())
    {
      // dont know anything about fareMarket->travelSeg() due to duplicates in FCO
      _softPass = true;
      return true;
    }
    else
      return validateCarrierRestrictionFare(trx, paxTypeFare, salesRestriction, isWQTrx);
  }

  if (salesRestriction->carrierSegInd() == RuleConst::NO_SEGMENT_OF_TICKET)
  {
    if (PricingTrx::MIP_TRX == trx.getTrxType() && fU == nullptr &&
        (paxTypeFare.fareMarket()->hasDuplicates()))
    {
      if (!validateCarrierRestrictionSegment(trx, itin, paxTypeFare, salesRestriction, isWQTrx))
        return false;

      _softPass = true;
    }
    else
      return validateCarrierRestrictionSegment(trx, itin, paxTypeFare, salesRestriction, isWQTrx);
  }

  // There's no restriction to this fare at this point
  return true;
}

//----------------------------------------------------------------------------
// checkSelectSecurity()
//----------------------------------------------------------------------------
bool
SalesRestrictionRule::checkSelectSecurity(const PaxTypeFare& paxTypeFare,
                                          const SalesRestriction* salesRestriction) const
{
  if (UNLIKELY(salesRestriction->tvlAgentSelectedInd() != RuleConst::BLANK)) // Any restriction ?
  {
    if (paxTypeFare.tcrTariffCat() == RuleConst::PRIVATE_TARIFF &&
        salesRestriction->overrideDateTblItemNo() != 0 && // No other data coded
        salesRestriction->countryRest() == RuleConst::BLANK &&
        salesRestriction->residentRest() == RuleConst::BLANK &&
        salesRestriction->carrierCrsInd() == RuleConst::BLANK &&
        salesRestriction->validationInd() == RuleConst::BLANK &&
        salesRestriction->carrierSegInd() == RuleConst::BLANK &&
        salesRestriction->tvlAgentSaleInd() == RuleConst::BLANK &&
        salesRestriction->fopCashInd() == RuleConst::BLANK &&
        salesRestriction->fopCheckInd() == RuleConst::BLANK &&
        salesRestriction->fopCreditInd() == RuleConst::BLANK &&
        salesRestriction->fopGtrInd() == RuleConst::BLANK &&
        salesRestriction->currCntryInd() == RuleConst::BLANK && salesRestriction->curr().empty() &&
        salesRestriction->tktIssMail() == RuleConst::BLANK &&
        salesRestriction->tktIssPta() == RuleConst::BLANK &&
        salesRestriction->tktIssMech() == RuleConst::BLANK &&
        salesRestriction->tktIssSelf() == RuleConst::BLANK &&
        salesRestriction->tktIssPtaTkt() == RuleConst::BLANK &&
        salesRestriction->tktIssAuto() == RuleConst::BLANK &&
        salesRestriction->tktIssSat() == RuleConst::BLANK &&
        salesRestriction->tktIssSatOcAto() == RuleConst::BLANK &&
        salesRestriction->tktIssElectronic() == RuleConst::BLANK &&
        salesRestriction->tktIssSiti() == RuleConst::BLANK &&
        salesRestriction->tktIssSoto() == RuleConst::BLANK &&
        salesRestriction->tktIssSito() == RuleConst::BLANK &&
        salesRestriction->tktIssSoti() == RuleConst::BLANK &&
        salesRestriction->familyGrpInd() == RuleConst::BLANK &&
        salesRestriction->extendInd() == RuleConst::BLANK &&
        salesRestriction->locales().size() == 0)
    {
      return false;
    }
  }
  return true;
}

namespace
{
inline bool
doesLocMatchCorpId(const PseudoCityCode& pcc,
                   const LocKey& locKey,
                   const PosPaxType* posPaxType,
                   bool matchedCorpId)
{
  return ((locKey.locType() == RuleConst::TRAVEL_AGENCY ||
           locKey.locType() == RuleConst::HOME_TRAVEL_AGENCY) &&
          locKey.loc() == pcc && matchedCorpId == !posPaxType->corpID().empty());
}
}

bool
SalesRestrictionRule::findCorpIdMatch(const PseudoCityCode& pcc,
                                      const std::vector<Locale*>& locales,
                                      const PosPaxType* posPaxType,
                                      bool matchedCorpId) const
{
  auto doesCorpIdMatch = [=](const Locale* locale) -> bool
  {
    return doesLocMatchCorpId(pcc, locale->loc1(), posPaxType, matchedCorpId) ||
           doesLocMatchCorpId(pcc, locale->loc2(), posPaxType, matchedCorpId);
  };
  return std::any_of(locales.cbegin(), locales.cend(), doesCorpIdMatch);
}

bool
SalesRestrictionRule::checkLocations(const LocKey& locKey1,
                                     const LocKey& locKey2,
                                     const Loc* locPO,
                                     const PricingTrx& trx,
                                     const PaxTypeFare& paxTypeFare) const
{
  return ((locKey1.locType() != RuleConst::BLANK && !locKey1.loc().empty() &&
           LocUtil::isInLoc((*locPO),
                            locKey1.locType(),
                            locKey1.loc(),
                            paxTypeFare.vendor(),
                            RESERVED,
                            LocUtil::OTHER,
                            GeoTravelType::International,
                            EMPTY_STRING(),
                            trx.getRequest()->ticketingDT())) ||
          (locKey2.locType() != RuleConst::BLANK && !locKey2.loc().empty() &&
           LocUtil::isInLoc((*locPO),
                            locKey2.locType(),
                            locKey2.loc(),
                            paxTypeFare.vendor(),
                            RESERVED,
                            LocUtil::OTHER,
                            GeoTravelType::International,
                            EMPTY_STRING(),
                            trx.getRequest()->ticketingDT())));
}

bool
SalesRestrictionRule::isOptIn(const LocCode& loc1, const LocCode& loc2) const
{
  return (loc1 == OPT_IN_AGENCY) || (loc2 == OPT_IN_AGENCY);
}

bool
SalesRestrictionRule::SalesRestrictionRule::isAgentOptIn(const Agent& agent) const
{
  const Customer* agentTJR = agent.agentTJR();
  if (agentTJR != nullptr)
    return (agentTJR->optInAgency() == YES);

  return false;
}

bool
SalesRestrictionRule::checkRussianGroup(const Loc& loc1, const Loc& loc2) const
{
  return LocUtil::isRussianGroup(loc1) && LocUtil::isRussianGroup(loc2);
}

bool
SalesRestrictionRule::checkUsTerritoryRule(const Loc& loc1, const Loc& loc2) const
{
  return LocUtil::isUSTerritoryRule(loc1) && LocUtil::isUSTerritoryRule(loc2);
}

bool
SalesRestrictionRule::checkScandinavia(const Loc& loc1, const Loc& loc2) const
{
  return LocUtil::isScandinavia(loc1) && LocUtil::isScandinavia(loc2);
}

const Loc*
SalesRestrictionRule::determineCountryLoc(const PricingTrx& trx) const
{
  if (!trx.getRequest()->salePointOverride().empty()) // Point of Sale overriden
    return getLoc(trx.getRequest()->salePointOverride(), trx);

  if (!trx.getRequest()->ticketPointOverride().empty()) // Point of Ticketing overriden
    return getLoc(trx.getRequest()->ticketPointOverride(), trx);

  return trx.getRequest()->ticketingAgent()->agentLocation();
}

const Loc*
SalesRestrictionRule::getLoc(const LocCode& loc, const PricingTrx& trx) const
{
  return trx.dataHandle().getLoc(loc, trx.getRequest()->ticketingDT());
}

const Nation*
SalesRestrictionRule::getNation(const NationCode& nation, const DateTime& dt, const PricingTrx& trx)
    const
{
  return trx.dataHandle().getNation(nation, dt);
}

const Agent*
SalesRestrictionRule::getAgent(const PricingTrx& trx, bool isQualifiedCategory) const
{
  if (UNLIKELY(trx.excTrxType() == PricingTrx::AF_EXC_TRX)) // &&refundQualifyingSecurity(trx,
                                                             // isQualifiedCategory))
    return static_cast<const RexBaseRequest*>(trx.getRequest())->currentTicketingAgent();

  return trx.getRequest()->ticketingAgent();
}

SalesRestrictionRule::Diag355::Diag355(PricingTrx& trx,
                                       const SalesRestriction* salesRestriction,
                                       const CategoryRuleItemInfo* rule,
                                       const PaxTypeFare& paxTypeFare)
  : _diagEnabled(false),
    _factory(nullptr),
    _diagPtr(nullptr),
    _trx(&trx),
    _salesRestriction(salesRestriction),
    _rule(rule),
    _paxTypeFare(&paxTypeFare)
{
  if (UNLIKELY(_trx->diagnostic().diagnosticType() == Diagnostic355))
  {
    _factory = DCFactory::instance();
    _diagPtr = _factory->create(*_trx);
    _diagPtr->enable(Diagnostic355);
    _diagEnabled = true;

    (*_diagPtr) << std::setw(3) << _paxTypeFare->fareMarket()->origin()->loc() << "-"
                << std::setw(15) << _paxTypeFare->fareMarket()->destination()->loc()
                << std::setw(10) << _paxTypeFare->fare()->carrier() << std::setw(10)
                << _paxTypeFare->fare()->fareClass() << std::endl;
  }
}

SalesRestrictionRule::Diag355::~Diag355()
{
}

void
SalesRestrictionRule::Diag355::printFailMsgAgencyInfo()
{
  if (UNLIKELY(_diagEnabled))
  {
    (*_diagPtr) << "FAILED SALES SECURITY " << std::endl << "TRX TVLAGENCYPCC - "
                << _trx->getRequest()->ticketingAgent()->tvlAgencyPCC() << "  TRX TVLAGENCYIATA - "
                << _trx->getRequest()->ticketingAgent()->tvlAgencyIATA() << std::endl
                << "CAT 15 TVLAGENTSALEIND - " << _salesRestriction->tvlAgentSaleInd()
                << "  CAT 15 CARRIERCRSIND - " << _salesRestriction->carrierCrsInd() << std::endl;
    _diagPtr->flushMsg();
  }
}

void
SalesRestrictionRule::Diag355::printFailMsgRelationalAndCarriers()
{
  if (UNLIKELY(_diagEnabled))
  {
    (*_diagPtr) << "FAILED SALES SECURITY " << std::endl << "RELATIONALIND - "
                << _rule->relationalInd() << "  ISFOOTNOTE - " << false << "  TRX PARTITIONID - "
                << _trx->billing()->partitionID() << "  FARE CARRIER - " << _paxTypeFare->carrier()
                << "  CAT 15 CARRIERCRSIND - " << _salesRestriction->carrierCrsInd()
                << "  CAT 15 OTHERCARRIER - " << _salesRestriction->otherCarrier() << std::endl;
    _diagPtr->flushMsg();
  } // Partition carrier is not the carrier
}

void
SalesRestrictionRule::Diag355::printFailMsgTvlAgentSaleInd()
{
  if (UNLIKELY(_diagEnabled))
  {
    (*_diagPtr) << "FAILED SALES SECURITY " << std::endl << "RELATIONALIND - "
                << _rule->relationalInd() << "  ISFOOTNOTE - " << false
                << "  CAT 15 TVLAGENTSALEIND - " << _salesRestriction->tvlAgentSaleInd()
                << std::endl;
    _diagPtr->flushMsg();
  }
}

void
SalesRestrictionRule::Diag355::printPassMsg()
{
  if (UNLIKELY(_diagEnabled))
  {
    (*_diagPtr) << "PASSED SALES SECURITY " << std::endl;
    _diagPtr->flushMsg();
  }
}

void
SalesRestrictionRule::Diag355::printFailMsgCarriers()
{
  if (UNLIKELY(_diagEnabled))
  {
    (*_diagPtr) << "FAILED SALES SECURITY " << std::endl << "CAT 15 CARRIERCRSIND - "
                << _salesRestriction->carrierCrsInd() << "  CAT 15 OTHERCARRIER - "
                << _salesRestriction->otherCarrier() << std::endl;
    _diagPtr->flushMsg();
  }
}

void
SalesRestrictionRule::Diag355::printFailMsgPartitionId()
{
  if (UNLIKELY(_diagEnabled))
  {
    (*_diagPtr) << "FAILED SALES SECURITY " << std::endl << "RELATIONALIND - "
                << _rule->relationalInd() << "  ISFOOTNOTE - " << false << "  FARE TCRTARIFFCAT - "
                << _paxTypeFare->tcrTariffCat() << "  TRX PARTITIONID - "
                << _trx->billing()->partitionID() << "  FARE CARRIER - " << _paxTypeFare->carrier()
                << "  CAT 15 OTHERCARRIER - " << _salesRestriction->otherCarrier() << std::endl;
    _diagPtr->flushMsg();
  }
} // Partition carrier is neither the publishing

bool
SalesRestrictionRule::isPropperGDS(const SalesRestriction* sr) const
{
  if (sr->carrierCrsInd() == RuleConst::MUST_BE_SOLD_VIA_CRS)
  {
    return (
        (sr->otherCarrier() == RuleConst::SABRE1W) || (sr->otherCarrier() == RuleConst::SABRE1S) ||
        (sr->otherCarrier() == RuleConst::SABRE1B) || (sr->otherCarrier() == RuleConst::SABRE1F) ||
        (sr->otherCarrier() == RuleConst::SABRE1J));
  }
  return true;
}

bool
SalesRestrictionRule::matchGDS(const SalesRestriction* sr, const Agent* agent) const
{
  return !((sr->otherCarrier() == RuleConst::SABRE1B && !(agent->abacusUser())) ||
           (sr->otherCarrier() == RuleConst::SABRE1F && !(agent->infiniUser())) ||
           (sr->otherCarrier() == RuleConst::SABRE1J && !(agent->axessUser())));
}

bool
SalesRestrictionRule::matchDeptCodeOAC(const PricingTrx& trx,
                                       const Agent& agent,
                                       const LocKey& locKey1,
                                       const LocKey& locKey2) const
{
  bool match(false);

  if (UNLIKELY(
          (agent.airlineDept().size() == 3 &&
           ((locKey1.loc() == agent.airlineDept()) || (locKey2.loc() == agent.airlineDept()))) ||
          (agent.officeDesignator().size() == 5 && ((locKey1.loc() == agent.officeDesignator()) ||
                                                    (locKey2.loc() == agent.officeDesignator())))))
  {
    match = true;
  }

  return match;
}

bool
SalesRestrictionRule::getResultForDFF(const PricingTrx& trx,
                                      const FareUsage* fu,
                                      const PaxTypeFare& ptf) const
{
  bool match(false);

  if (ptf.vendor().equalToConst("ATP") || ptf.vendor().equalToConst("SITA") || ptf.vendor().equalToConst("SMFA") ||
      ptf.vendor().equalToConst("SMFC"))
  {
    if (fu == nullptr)
    {
      if (ptf.cat25Fare() && RuleUtil::isVendorPCC(ptf.cat25Fare()->vendor(), trx))
      {
        if (ptf.cat25Fare()->fcaDisplayCatType() == RuleConst::SELLING_FARE ||
            ptf.cat25Fare()->fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE ||
            ptf.cat25Fare()->fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE_UPD)
          match = true;
      }
    }
    else
    {
      if (fu->cat25Fare() && RuleUtil::isVendorPCC(fu->cat25Fare()->vendor(), trx))
      {
        if (fu->cat25Fare()->fcaDisplayCatType() == RuleConst::SELLING_FARE ||
            fu->cat25Fare()->fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE ||
            fu->cat25Fare()->fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE_UPD)
          match = true;
      }
    }
  }
  return match;
}

bool
SalesRestrictionRule::validateAllCarriersIS(PricingTrx& trx,
                                            PaxTypeFare& paxTypeFare,
                                            Itin& itin,
                                            const SalesRestriction* salesRestriction) const
{
  DiagCollector* diag = 0;
  if (UNLIKELY(trx.diagnostic().isActive() &&
                (trx.diagnostic().diagnosticType() == Diagnostic191)))
  {
    DCFactory* factory = DCFactory::instance();
    diag = factory->create(trx);
    diag->enable(Diagnostic191);
    *diag << "\nFARE " << paxTypeFare.fare()->fareClass() << ": ";
  }

  std::vector<CarrierCode> itinVCs;
  itin.getValidatingCarriers(trx, itinVCs);

  if (itinVCs.empty() && !itin.validatingCarrier().empty())
    itinVCs.push_back(itin.validatingCarrier());

  uint16_t passedVCCount(0);
  for (const CarrierCode& vcr : itinVCs)
  {
    if (validateCarrier(trx, vcr, paxTypeFare, salesRestriction, false))
      ++passedVCCount;
    else if (UNLIKELY(diag))
      *diag << "VALIDATING CARRIER " << vcr << " FAILED CAT15\n";
  }

  if (UNLIKELY(diag))
  {
    if (passedVCCount == itinVCs.size())
      *diag << "ALL VALIDATING CARRIERS PASSED CAT15\n";
    else if (passedVCCount == 0)
      *diag << "NO VALIDATING CARRIERS PASSED CAT15\n";

    diag->flushMsg();
  }

  return passedVCCount;
}

bool
SalesRestrictionRule::checkCarrierMatch(const CarrierCode& cxr,
                                        const CarrierCode& restriction,
                                        const PricingTrx& trx) const
{
  if (restriction.empty())
    return false;
  return CarrierUtil::carrierExactOrAllianceMatch(cxr, restriction, trx);
}

class SalesRestrictionObservers
{
public:
  SalesRestrictionObservers(FareUsage* fu,
                            PaxTypeFare& ptf,
                            PricingTrx& trx,
                            SalesRestrictionRule& rule)
    : _fu(fu), _ptf(ptf)
  {
    if (trx.getRequest() && trx.getRequest()->isSFR())
    {
      if (fu)
      {
        _fareUsageObservers.push_back(AdvanceResAndTktObserverType<FareUsage>::create(
            ADVANCE_RESERVATION_SFR, trx.dataHandle(), &rule));
        _fareUsageObservers.push_back(AdvanceResAndTktObserverType<FareUsage>::create(
            ADVANCE_TICKETING_SFR, trx.dataHandle(), &rule));
      }
      else
      {
        _paxTypeFareObservers.push_back(AdvanceResAndTktObserverType<PaxTypeFare>::create(
            ADVANCE_RESERVATION_SFR, trx.dataHandle(), &rule));
        _paxTypeFareObservers.push_back(AdvanceResAndTktObserverType<PaxTypeFare>::create(
            ADVANCE_TICKETING_SFR, trx.dataHandle(), &rule));
      }
    }
  }

  void updateIfNotified()
  {
    for (auto& observer : _paxTypeFareObservers)
      observer->updateIfNotified(_ptf);
    for (auto& observer : _fareUsageObservers)
      observer->updateIfNotified(*_fu);
  }

private:
  std::vector<std::unique_ptr<AdvanceResAndTktObserverType<PaxTypeFare>>> _paxTypeFareObservers;
  std::vector<std::unique_ptr<AdvanceResAndTktObserverType<FareUsage>>> _fareUsageObservers;
  FareUsage* _fu;
  PaxTypeFare& _ptf;
};

Record3ReturnTypes
SalesRestrictionRuleWrapper::validate(PricingTrx& trx,
                                      Itin& itin,
                                      FareUsage* fareUsage,
                                      PaxTypeFare& paxTypeFare,
                                      const CategoryRuleInfo& cri,
                                      const CategoryRuleItemInfo* rule,
                                      const SalesRestriction* salesRestriction,
                                      bool isQualifiedCategory,
                                      bool& isCat15Security,
                                      bool skipCat15Security)
{
  SalesRestrictionObservers observers(fareUsage, paxTypeFare, trx, _rule);

  const Record3ReturnTypes retval = _rule.validate(trx,
                                                   itin,
                                                   fareUsage,
                                                   paxTypeFare,
                                                   cri,
                                                   rule,
                                                   salesRestriction,
                                                   isQualifiedCategory,
                                                   isCat15Security,
                                                   skipCat15Security);

  observers.updateIfNotified();

  return retval;
}

Record3ReturnTypes
SalesRestrictionRuleWrapper::validate(PricingTrx& trx,
                                      Itin& itin,
                                      FareUsage* fareUsage,
                                      PaxTypeFare& paxTypeFare,
                                      const CombinabilityRuleInfo& cri,
                                      const CombinabilityRuleItemInfo* rule,
                                      const SalesRestriction* salesRestriction,
                                      bool isQualifiedCategory,
                                      bool& isCat15Security,
                                      bool skipCat15Security)
{
  SalesRestrictionObservers observers(fareUsage, paxTypeFare, trx, _rule);

  const Record3ReturnTypes retval = _rule.validate(trx,
                                                   itin,
                                                   fareUsage,
                                                   paxTypeFare,
                                                   cri,
                                                   rule,
                                                   salesRestriction,
                                                   isQualifiedCategory,
                                                   isCat15Security,
                                                   skipCat15Security);

  observers.updateIfNotified();

  return retval;
}

void
SalesRestrictionRuleWrapper::setRuleDataAccess(RuleControllerDataAccess* ruleDataAccess)
{
  _rule.setRuleDataAccess(ruleDataAccess);
}
} // end namespace tse
