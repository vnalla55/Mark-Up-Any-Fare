//----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------

#include "Rules/MinimumStayApplication.h"

#include "Common/FallbackUtil.h"
#include "Common/ItinUtil.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/RtwUtil.h"
#include "Common/TravelSegUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/ShoppingTrx.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/MinStayRestriction.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagManager.h"
#include "Diagnostic/Diag306Collector.h"
#include "Rules/MinMaxRulesUtils.h"
#include "Rules/IObserver.h"
#include "Rules/PeriodOfStay.h"
#include "Rules/RuleUtil.h"
#include "Rules/RuleValidationChancelor.h"
#include "Util/BranchPrediction.h"

#include <algorithm>

namespace tse
{
FIXEDFALLBACK_DECL(APO29538_StopoverMinStay)

static Logger
logger("atseintl.Rules.MinimumStayApplication");
const std::string MinimumStayApplication::NO_MINIMUM_STAY = "000";
const std::string MinimumStayApplication::HOURS = "H";
const std::string MinimumStayApplication::MINUTES = "N";

namespace
{
Diag306Collector*
prepare306Diag(PricingTrx& trx)
{
  DCFactory* factory = DCFactory::instance();
  DiagCollector* diagPtr = factory->create(trx);
  Diag306Collector* diag306 = dynamic_cast<Diag306Collector*>(diagPtr);
  if (diag306 && diagPtr->diagnosticType() == Diagnostic306)
    diag306->enable(Diagnostic306);
  return diag306;
}
inline void
printDiagData(Diag306Collector* diag,
              Diag306Type diag306Type,
              const MinStayRestriction* minStayRule = nullptr)
{
  if (UNLIKELY(diag))
    diag->printDiagData(diag306Type, minStayRule);
}

inline void
formatDiagnostic(Diag306Collector* diag, const TravelSeg* travelSeg, Record3ReturnTypes retval)
{
  if (UNLIKELY(diag))
    diag->formatDiagnostic(travelSeg, retval);
}

inline void
displayMinStayRules(Diag306Collector* diag,
                    const MinStayRestriction& minStayRule,
                    const PeriodOfStay& minStayPeriod)
{
  if (UNLIKELY(diag))
    diag->displayMinStayRules(minStayRule, minStayPeriod);
}

inline void
displayReturnDates(Diag306Collector* diag, const DateTime& minStayPeriodReturnDate)
{
  if (UNLIKELY(diag))
    diag->displayReturnDates(minStayPeriodReturnDate);
}
inline void
displayDeterminedMinStayDate(Diag306Collector* diag, const DateTime& minStayPeriodReturnDate)
{
  if (UNLIKELY(diag))
    diag->displayDeterminedMinStayDate(minStayPeriodReturnDate);
}
} // anonymous namespace

bool
MinimumStayApplication::nonFlexFareValidationNeeded(PricingTrx& trx) const
{
  return (!trx.isFlexFare() && trx.getOptions()->isNoMinMaxStayRestr());
}

void
MinimumStayApplication::printDataUnavailableFailed(Diag306Collector* diag,
                                                   Record3ReturnTypes retVal,
                                                   const MinStayRestriction* minStayRule)
{
  LOG4CXX_DEBUG(logger, "Data Unavailable failed");
  if (UNLIKELY(diag))
    diag->printDataUnavailableFailed(retVal, minStayRule);
}

Record3ReturnTypes
MinimumStayApplication::validate(PricingTrx& trx,
                                 Itin& itin,
                                 const PaxTypeFare& paxTypeFare,
                                 const RuleItemInfo* rule,
                                 const FareMarket& fareMarket)
{
  if (UNLIKELY(PricingTrx::ESV_TRX == trx.getTrxType()))
  {
    return validateEsv(static_cast<ShoppingTrx&>(trx), paxTypeFare, rule, fareMarket);
  }

  LOG4CXX_INFO(logger, " Entered MinimumStayApplication::validate()");

  LOG4CXX_DEBUG(logger, "Faremarket direction = " << fareMarket.getDirectionAsString());

  const MinStayRestriction* minStayRule = dynamic_cast<const MinStayRestriction*>(rule);

  if (UNLIKELY(!minStayRule))
    return SKIP;

  Diag306Collector* diag = prepare306Diag(trx);
  Record3ReturnTypes retVal = validateUnavailableDataTag(minStayRule->unavailTag());

  if (retVal != PASS)
  {
    printDataUnavailableFailed(diag, retVal, minStayRule);
    return retVal;
  }

  //-------------------------------------------------
  // Added for SITA v1.1
  // Value "000" of minStay means No Minimum Stay
  //-------------------------------------------------
  if (minStayRule->minStay() == NO_MINIMUM_STAY)
  {
    printDiagData(diag, Diag306Type::NO_MINIMUM_STAY);
    return PASS;
  }

  // Check if Request wants to accept fares
  // with MinStay restriction
  const bool nonFlexFareValidation = nonFlexFareValidationNeeded(trx);
  if (nonFlexFareValidation || isValidationNeeded(RuleConst::MINIMUM_STAY_RULE, trx))
  {
    printDiagData(diag, Diag306Type::MIN_STAY_FAILED);
    updateStatus(RuleConst::MINIMUM_STAY_RULE, FAIL);
    if (nonFlexFareValidation || shouldReturn(RuleConst::MINIMUM_STAY_RULE))
      return FAIL;
  }

  if (softpassFareComponentPhase(trx, itin, fareMarket, paxTypeFare, diag))
    return SOFTPASS;
  // apo-38902:check if outbound fc has To tsi  of pu scope.
  // bypass this check for IS transactions
  bool outboundToTSIisPUScope = false;
  if ( (PricingTrx::IS_TRX != trx.getTrxType()))
  {
    int geoTblItemNum = 0;
    RuleConst::TSIScopeType tsiScope;
    // the fc is outbound as all others fcs are softpassed
    geoTblItemNum = minStayRule->geoTblItemNoTo();
    if (geoTblItemNum)
    {
      if (!RuleUtil::getTSIScopeFromGeoRuleItem(geoTblItemNum, paxTypeFare.vendor(), trx, tsiScope))
      {
        printDiagData(diag, Diag306Type::FAIL_FARE_COMP);
        return FAIL;
      }
      if ((tsiScope == RuleConst::TSI_SCOPE_JOURNEY) ||
          (tsiScope == RuleConst::TSI_SCOPE_SUB_JOURNEY))
        outboundToTSIisPUScope = true;
    }
  } // fallback

  std::vector<TravelSeg*> applTravelSegment;

  if ((itin.travelSeg().size() == size_t(itin.segmentOrder(fareMarket.travelSeg().back()))) &&
      (fareMarket.travelSeg().size() == 1))
  {
    LOG4CXX_DEBUG(logger, "Faremarket ending segment is last travel seg in itin");
    return PASS;
  }

  NoPNRPricingTrx* noPnrTrx = dynamic_cast<NoPNRPricingTrx*>(&trx);
  // if NoPNR transaction with all dates empty - autopass with warning
  if (UNLIKELY(noPnrTrx != nullptr && itin.dateType() == Itin::NoDate))
  {
    // autopass the rule
    LOG4CXX_INFO(logger, " AUTOPASS OPEN SEGMENT - FOR WQ TRX [1]");
    return PASS;
  }
  // Open segment:
  // Pass rule
  //  If the 1st travelSeg in FC is open and
  //   there are no more a dated TravelSegs in Itin.
  //  otherwise, validate as ussualy with date+1(done in ItinAnalizer).
  // Do not autopass this way, if NoPNR (WQ) transaction
  TravelSeg* frontTrSeg = fareMarket.travelSeg().front();
  if (UNLIKELY(frontTrSeg != nullptr && noPnrTrx == nullptr && frontTrSeg->isOpenWithoutDate() &&
                ItinUtil::isOpenSegAfterDatedSeg(itin, frontTrSeg)))
  {
    printDiagData(diag, Diag306Type::PASS_OPEN_SEGMENT);
    return SOFTPASS;
  }

  PeriodOfStay minStayPeriod(minStayRule->minStay(), minStayRule->minStayUnit());

  DateTime dteToCheck = frontTrSeg->departureDT();
  if (UNLIKELY(noPnrTrx != nullptr))
  {
    noPnrTrx->updateOpenDateIfNeccesary(frontTrSeg, dteToCheck);
  }

  const DateTime minStayPeriodReturnDate = RuleUtil::addPeriodToDate(dteToCheck, minStayPeriod);

  const DateTime earliestReturnDate =
      determineEarliestReturnDate(minStayPeriodReturnDate, minStayRule, minStayPeriod, diag);

  dteToCheck = itin.travelSeg().front()->departureDT();
  if (UNLIKELY(noPnrTrx != nullptr))
  {
    noPnrTrx->updateOpenDateIfNeccesary(itin.travelSeg().front(), dteToCheck);
  }

  const DateTime minStayPeriodReturnDateForItin =
      RuleUtil::addPeriodToDate(dteToCheck, minStayPeriod);
  const DateTime earliestReturnDateForItin =
      determineEarliestReturnDate(minStayPeriodReturnDateForItin, minStayRule, minStayPeriod);

  if (LIKELY(!itin.travelSeg().empty()))
  {
    LOG4CXX_DEBUG(
        logger,
        "start faremarket segment number: " << itin.segmentOrder(fareMarket.travelSeg().front()));
    LOG4CXX_DEBUG(
        logger,
        "end faremarket segment number: " << itin.segmentOrder(fareMarket.travelSeg().back()));
    LOG4CXX_DEBUG(logger, "itin travel seg size: " << itin.travelSeg().size());
    LOG4CXX_DEBUG(logger, "itin faremarket size: " << itin.fareMarket().size());

    for (size_t i = itin.segmentOrder(fareMarket.travelSeg().front()); i < itin.travelSeg().size();
         i++)
    {
      TravelSeg* travelSeg = itin.travelSeg()[i];
      applTravelSegment.push_back(travelSeg);
    }
  }
  else
  {
    printDiagData(diag, Diag306Type::FAIL_ITIN_TVL_SEG_EMPTY);
    return FAIL;
  }

  LOG4CXX_DEBUG(logger, "Travel seg size = " << applTravelSegment.size());

  // We only want to validate the origin DOW if there
  // is one travel segment
  //
  if (!minStayRule->originDow().empty() && fareMarket.travelSeg().size() > 1)
  {
    return SOFTPASS;
  }

  if ((trx.getTrxType() == PricingTrx::MIP_TRX) && (trx.isAltDates()))
  {
    TravelSeg* travelSeg = applTravelSegment.back();
    for (Itin* itin : trx.itin())
    {
      if (LIKELY(!itin->travelSeg().empty()))
      {
        TravelSeg* tvlSeg = itin->travelSeg().back();
        if (tvlSeg->departureDT() > travelSeg->departureDT())
          travelSeg = tvlSeg;
      }
    }
    if (travelSeg != applTravelSegment.back())
    {
      applTravelSegment.push_back(travelSeg);
    }
  }

  std::vector<TravelSeg*>::reverse_iterator applTravelSegmentI = applTravelSegment.rbegin();
  std::vector<TravelSeg*>::reverse_iterator travelSegEnd = applTravelSegment.rend();
  // loop through the travel segs and validate origin day of week
  //
  int failCount = 0;
  int passCount = 0;

  for (; applTravelSegmentI != travelSegEnd; applTravelSegmentI++)
  {
    DateTime travelDateTime = fareMarket.travelSeg()[0]->departureDT();
    if (UNLIKELY(noPnrTrx != nullptr))
    {
      noPnrTrx->updateOpenDateIfNeccesary(fareMarket.travelSeg()[0], travelDateTime);
    }

    uint32_t travelDOW = travelDateTime.date().day_of_week().as_number();

    LOG4CXX_DEBUG(logger, "Travel dow = " << travelDOW);

    if (travelDOW == 0)
      travelDOW = 7;

    uint32_t ruleDOW = 0;
    retVal = validateOriginDOW(minStayRule, travelDOW, ruleDOW);

    if (retVal == FAIL)
    {
      printDiagData(diag, Diag306Type::ORIGIN_DAY_OF_WEEK_FAIL);
      formatDiagnostic(diag, *applTravelSegmentI, retVal);
      return SKIP;
    }

    LOG4CXX_DEBUG(logger, "Validating Earliest Departure Date");
    LOG4CXX_DEBUG(logger,
                  "Calculated Earliest Departure Date " << earliestReturnDate.toSimpleString());
    LOG4CXX_DEBUG(logger, "MinStayDate from Rule " << minStayRule->minStayDate().toSimpleString());
    LOG4CXX_DEBUG(logger,
                  "Travel Seg Dep Date " << (*applTravelSegmentI)->departureDT().toSimpleString());
    LOG4CXX_DEBUG(logger,
                  "Travel Seg Arr Date " << (*applTravelSegmentI)->departureDT().toSimpleString());

    LOG4CXX_DEBUG(logger,
                  "Orig. Departure Date " << (*applTravelSegmentI)->departureDT().toSimpleString());

    if (UNLIKELY((*applTravelSegmentI)->isOpenWithoutDate() && noPnrTrx == nullptr &&
                  ItinUtil::isOpenSegAfterDatedSeg(itin, *applTravelSegmentI)))
    {
      printDiagData(diag, Diag306Type::PASS_OPEN_SEGMENT);
      // we still need to set NVB
      retVal = PASS; // Job is done. The current travelSeg is open and the last in ITIN.
    }
    else
    {
      DateTime tmpDepartureDate = (*applTravelSegmentI)->departureDT();
      if (UNLIKELY(noPnrTrx != nullptr))
      {
        noPnrTrx->updateOpenDateIfNeccesary((*applTravelSegmentI), tmpDepartureDate);
      }

      if (itin.travelSeg().size() == fareMarket.travelSeg().size())
      {
        retVal = validateReturnDate(tmpDepartureDate, earliestReturnDate);
      }
      else
      {
        retVal = validateReturnDate(tmpDepartureDate, earliestReturnDateForItin);
      }
    }
    //APO38902 if  IS trx fails, leave it as fail. For other trxs softpass if tsi is pu scope
    if ((retVal == FAIL) &&
        (PricingTrx::IS_TRX != trx.getTrxType()))
    {
      // if we have failed, but tsi is pu scope then softpass
      if (outboundToTSIisPUScope)
        retVal = SOFTPASS;
    }

    formatDiagnostic(diag, *applTravelSegmentI, retVal);

    if (retVal == PASS)
    {
      passCount++;
    }
    else if (LIKELY(retVal == FAIL))
      failCount++;
  }

  if ((passCount > 0) && (failCount > 0))
  {
    printDiagData(diag, Diag306Type::SOFTPASS_NEED_REVALIDATION_FOR_PU);
    retVal = SOFTPASS;
  }

  //APO38902 do not do this check for IS trx
  if (PricingTrx::IS_TRX != trx.getTrxType())
  {
    // if we have failed, but tsi is pu scope then softpass
    if ((retVal == FAIL) && outboundToTSIisPUScope)
      retVal = SOFTPASS;
  }
  if ((retVal == FAIL) && (!minStayPeriod.isDayOfWeek()) &&
      (PricingTrx::IS_TRX == trx.getTrxType()))
  {
    // check if it SOL that way to avoid dynamic_cast<ShoppingTrx*>
    if (fareMarket.getApplicableSOPs())
    {
      if (PASS == validateReturnDate(itin.getMaxDepartureDT(), earliestReturnDateForItin))
      {
        retVal = SOFTPASS;
      }
    }
  }

  return retVal;
}

Record3ReturnTypes
MinimumStayApplication::validateEsv(ShoppingTrx& trx,
                                    const PaxTypeFare& paxTypeFare,
                                    const RuleItemInfo* rule,
                                    const FareMarket& fareMarket)
{
  LOG4CXX_DEBUG(logger, "MinimumStayApplication::validateEsv");
  Record3ReturnTypes retVal = SKIP;

  const MinStayRestriction* minStayRule = dynamic_cast<const MinStayRestriction*>(rule);

  if (!minStayRule)
    return SKIP;

  retVal = validateUnavailableDataTag(minStayRule->unavailTag());

  if (retVal != PASS)
  {
    LOG4CXX_DEBUG(logger, "MinimumStayApplication::validateEsv - FAIL: Data Unavailable");
    return retVal;
  }

  if (NO_MINIMUM_STAY == minStayRule->minStay())
  {
    LOG4CXX_DEBUG(logger, "MinimumStayApplication::validateEsv - PASS: No MinStay");
    return PASS;
  }

  if (fareMarket.direction() != FMDirection::OUTBOUND)
  {
    LOG4CXX_DEBUG(
        logger, "MinimumStayApplication::validateEsv - SOFTPASS: Fare directionality not OUTBOUND");
    return SOFTPASS;
  }

  if ((paxTypeFare.owrt() == ONE_WAY_MAY_BE_DOUBLED) ||
      (paxTypeFare.owrt() == ONE_WAY_MAYNOT_BE_DOUBLED))
  {
    LOG4CXX_DEBUG(logger, "MinimumStayApplication::validateEsv - SOFTPASS: PaxType fare is ONEWAY");
    return SOFTPASS;
  }

  const DateTime& outDepartureDateTime = trx.journeyItin()->travelSeg().front()->departureDT();
  const DateTime& inDepartureDateTime = trx.journeyItin()->travelSeg().back()->departureDT();

  DateTime outDepartureDate(
      outDepartureDateTime.year(), outDepartureDateTime.month(), outDepartureDateTime.day());

  DateTime inDepartureDate(
      inDepartureDateTime.year(), inDepartureDateTime.month(), inDepartureDateTime.day());

  PeriodOfStay periodOfStay(minStayRule->minStay(), minStayRule->minStayUnit());

  if ((periodOfStay.unit() == PeriodOfStay::HOURS) ||
      (periodOfStay.unit() == PeriodOfStay::MINUTES))
  {
    inDepartureDate = inDepartureDate.addDays(1);
  }

  uint32_t travelDOW = outDepartureDate.date().day_of_week().as_number();

  if (0 == travelDOW)
  {
    travelDOW = 7;
  }

  uint32_t ruleDOW = 0;
  retVal = validateOriginDOW(minStayRule, travelDOW, ruleDOW);

  if (FAIL == retVal)
  {
    LOG4CXX_DEBUG(logger, "MinimumStayApplication::validateEsv - SKIP: Origin Day of Week failed");
    return SKIP;
  }

  const DateTime minStayPeriodReturnDate =
      RuleUtil::addPeriodToDate(outDepartureDate, periodOfStay);
  const DateTime earliestReturnDate = MinMaxRulesUtils::determineReturnDate(
      minStayPeriodReturnDate, minStayRule->minStayDate(), minStayRule->earlierLaterInd());

  if (earliestReturnDate > inDepartureDate)
    retVal = FAIL;
  else
    retVal = PASS;

  return retVal;
}

Record3ReturnTypes
MinimumStayApplication::validate(PricingTrx& trx,
                                 const RuleItemInfo* rule,
                                 const FarePath& farePath,
                                 const PricingUnit& pricingUnit,
                                 const FareUsage& fareUsage)
{
  if (!fallback::fixed::APO29538_StopoverMinStay())
  {
    LOG4CXX_INFO(logger, " Entered MinimumStayApplication::validate() for FarePath");
  }

  return validate(trx, rule, &farePath, *(farePath.itin()), pricingUnit, fareUsage);
}

Record3ReturnTypes
MinimumStayApplication::validate(PricingTrx& trx,
                                 const RuleItemInfo* rule,
                                 const Itin& itin,
                                 const PricingUnit& pricingUnit,
                                 const FareUsage& fareUsage)
{
  if (!fallback::fixed::APO29538_StopoverMinStay())
  {
    LOG4CXX_INFO(logger, " Entered MinimumStayApplication::validate() for PricingUnit");
  }

  return validate(trx, rule, nullptr, itin, pricingUnit, fareUsage);
}

Record3ReturnTypes
MinimumStayApplication::validate(PricingTrx& trx,
                                 const RuleItemInfo* rule,
                                 const FarePath* farePath,
                                 const Itin& itin,
                                 const PricingUnit& pricingUnit,
                                 const FareUsage& fareUsage)
{
  const MinStayRestriction* minStayRule = dynamic_cast<const MinStayRestriction*>(rule);

  if (UNLIKELY(!minStayRule))
    return FAIL;

  Diag306Collector* diag = prepare306Diag(trx);

  Record3ReturnTypes retVal = validateUnavailableDataTag(minStayRule->unavailTag());

  if (retVal != PASS)
  {
    printDataUnavailableFailed(diag, retVal, minStayRule);
    return retVal;
  }

  //-------------------------------------------------
  // Added for SITA v1.1
  // Value "000" of minStay means No Minimum Stay
  //-------------------------------------------------
  if (UNLIKELY(minStayRule->minStay() == NO_MINIMUM_STAY))
  {
    printDiagData(diag, Diag306Type::NO_MINIMUM_STAY);
    return PASS;
  }

  // Check if Request wants to accept fares
  // with MinStay restriction
  bool nonFlexFareValidation = nonFlexFareValidationNeeded(trx);
  if (UNLIKELY(nonFlexFareValidation || isValidationNeeded(RuleConst::MINIMUM_STAY_RULE, trx)))
  {
    printDiagData(diag, Diag306Type::MIN_STAY_FAILED);
    updateStatus(RuleConst::MINIMUM_STAY_RULE, FAIL);
    if (nonFlexFareValidation || shouldReturn(RuleConst::MINIMUM_STAY_RULE))
      return FAIL;
  }

  if (pricingUnit.puType() == PricingUnit::Type::ONEWAY)
  {
    printDiagData(diag, Diag306Type::PASS_ONEWAY);
    return PASS;
  }

  PeriodOfStay minStayPeriod(minStayRule->minStay(), minStayRule->minStayUnit());

  // If no Minimum Stay period or Minimum Stay Date then the rule
  // does not apply
  //
  if (UNLIKELY(!(minStayPeriod.isValid()) && (!(minStayRule->minStayDate().isValid()))))
    return SKIP;

  RuleUtil::TravelSegWrapperVector applTravelSegment;

  NoPNRPricingTrx* noPnrTrx = dynamic_cast<NoPNRPricingTrx*>(&trx);
  // set cat 6/7 warning (WQ)
  if (UNLIKELY(noPnrTrx != nullptr))
  {
    // set WQ rule warning
    fareUsage.paxTypeFare()->warningMap().set(WarningMap::cat6_warning);
  }

  if (UNLIKELY((minStayRule->geoTblItemNoFrom() == 0) || (minStayRule->geoTblItemNoTo() == 0)))
  {
    printDiagData(diag, Diag306Type::FAIL_GEO_TLB_ITEM, minStayRule);
    return FAIL;
  }

  displayMinStayRules(diag, *minStayRule, minStayPeriod);

  // validate table 995 if directed to by Minimum Stay Record
  //
  if (fallback::fixed::APO29538_StopoverMinStay())
  {
    retVal = getGeoTravelSegs(applTravelSegment,
                              minStayRule->geoTblItemNoFrom(),
                              trx,
                              fareUsage.paxTypeFare()->vendor(),
                              nullptr,
                              farePath,
                              itin,
                              pricingUnit,
                              true,
                              true);
  }
  else
  {
    retVal = getGeoTravelSegs(applTravelSegment,
                              minStayRule->geoTblItemNoFrom(),
                              trx,
                              fareUsage.paxTypeFare()->vendor(),
                              &fareUsage,
                              farePath,
                              itin,
                              pricingUnit,
                              true,
                              true);
  }

  if (retVal != PASS)
  {
    printDiagData(diag, Diag306Type::SKIP_GEO_RULE);
    return retVal;
  }

  const RuleUtil::TravelSegWrapper* tsw1 = applTravelSegment.front();
  const TravelSeg* travelSeg1 = tsw1->travelSeg();

  // check OPEN segment:
  // The rule should pass if the 1st travelSeg is open and
  //           there are no more a dated TravelSegs in Itin.
  //  otherwise, validate as ussualy with date+1(done in ItinAnalizer).
  //  doesn't apply to NoPNR (WQ) transactions
  if (UNLIKELY(travelSeg1 != nullptr && noPnrTrx == nullptr && travelSeg1->isOpenWithoutDate() &&
                travelSeg1->openSegAfterDatedSeg()))
  {
    printDiagData(diag, Diag306Type::PASS_OPEN_SEGMENT);
    return PASS; // Job is done. Do not set NVB.
  }

  DateTime fromDepDateTime = trx.transactionStartTime();

  if (tsw1->origMatch())
  {
    fromDepDateTime = travelSeg1->departureDT();
  }
  else if (LIKELY(tsw1->destMatch()))
  {
    fromDepDateTime = travelSeg1->arrivalDT();
  }

  uint32_t fromTravelDOW = 0;

  if (LIKELY(tsw1->destMatch() || tsw1->origMatch()))
  {
    if (UNLIKELY(noPnrTrx != nullptr && itin.dateType() != Itin::NoDate))
    {
      noPnrTrx->updateOpenDateIfNeccesary(travelSeg1, fromDepDateTime);
    }
    fromTravelDOW = fromDepDateTime.date().day_of_week().as_number();
  }

  bool noPnrWithOutDate = (noPnrTrx != nullptr && itin.dateType() == Itin::NoDate);

  if (fallback::fixed::APO29538_StopoverMinStay())
  {
    retVal = getGeoTravelSegs(applTravelSegment,
                              minStayRule->geoTblItemNoTo(),
                              trx,
                              fareUsage.paxTypeFare()->vendor(),
                              nullptr,
                              farePath,
                              itin,
                              pricingUnit,
                              true,
                              true);
  }
  else
  {
    retVal = getGeoTravelSegs(applTravelSegment,
                              minStayRule->geoTblItemNoTo(),
                              trx,
                              fareUsage.paxTypeFare()->vendor(),
                              &fareUsage,
                              farePath,
                              itin,
                              pricingUnit,
                              true,
                              true);
  }

  if (retVal != PASS)
  {
    if (noPnrWithOutDate)
      return PASS;

    printDiagData(diag, Diag306Type::SKIP_GEO_RULE);
    return retVal;
  }

  const RuleUtil::TravelSegWrapper* tsw2 = applTravelSegment.back();
  const TravelSeg* travelSeg2 = tsw2->travelSeg();

  const bool isRtw = trx.getOptions() && trx.getOptions()->isRtw();

  if (LIKELY(!isRtw))
  {
    if (UNLIKELY(
            removeGeoTravelSegs(applTravelSegment, fareUsage, pricingUnit, getTsiTo(), &trx) &&
            applTravelSegment.empty()))
    {
      if (noPnrWithOutDate)
        return PASS;

      printDiagData(diag, Diag306Type::SAME_GEO_IN_FARE_USAGE);
      return SKIP;
    }
  }

  if (fromTravelDOW == 0)
  {
    fromTravelDOW = 7;
  }

  uint32_t ruleDOW = 0;

  retVal = validateOriginDOW(minStayRule, fromTravelDOW, ruleDOW);

  if (retVal == FAIL && !noPnrWithOutDate)
  {
    printDiagData(diag, Diag306Type::ORIGIN_DAY_OF_WEEK_FAIL);
    return SKIP;
  }

  // Earliest return date is calculated using the minimum stay period and
  // unit of time , default value is negative infinity
  //
  const DateTime minStayPeriodReturnDate =
      RuleUtil::addPeriodToDate(fromDepDateTime, minStayPeriod);

  DateTime earliestReturnDate =
      determineEarliestReturnDate(minStayPeriodReturnDate, minStayRule, minStayPeriod, diag);

  if (noPnrWithOutDate)
  {
    // if NoPNR transaction with all dates empty - autopass with warning
    notifyObservers(minStayRule,
                    earliestReturnDate,
                    itin.segmentOrder(travelSeg2),
                    travelSeg2->boardMultiCity());

    LOG4CXX_INFO(logger, "AUTOPASS OPEN SEGMENT - FOR WQ TRX [2]");

    return PASS;
  }

  DateTime toDepDateTime = trx.transactionStartTime();

  // check OPEN segment:
  // The rule should pass if the current back travelSeg is open and
  //           there are no more a dated TravelSegs in Itin.
  //  otherwise, validate as ussualy with date+1(done in ItinAnalizer).
  //  doesn't apply to WQ (NoPNR) transactions
  if (UNLIKELY(travelSeg2->isOpenWithoutDate() && noPnrTrx == nullptr &&
                travelSeg2->openSegAfterDatedSeg()))
  {
    printDiagData(diag, Diag306Type::PASS_OPEN_SEGMENT);
    // still need NVB information
    notifyObservers(minStayRule,
                    earliestReturnDate,
                    itin.segmentOrder(travelSeg2),
                    travelSeg2->boardMultiCity());
    return PASS; // Job is done.
  }

  {
    const TravelSeg* tmpTravelSeg = travelSeg2;
    if (tsw2->origMatch())
      toDepDateTime = travelSeg2->departureDT();
    else if (tsw2->destMatch())
    {
      if (travelSeg1 != travelSeg2)
        toDepDateTime = travelSeg2->arrivalDT();
      else
      {
        // The last call to getGeoTravelSegs returned the same Travel Segment
        // as the first call. This can happen if the GEO TO is: Arrival At
        // First Point of Turnaround. We therefore must get the departure of
        // the last travel segment in this PU.
        //
        const TravelSeg* lastPuTravelSeg = pricingUnit.travelSeg().back();
        tmpTravelSeg = lastPuTravelSeg;
        if (lastPuTravelSeg)
          toDepDateTime = lastPuTravelSeg->departureDT();
      }
    }
    if (UNLIKELY(noPnrTrx != nullptr && tmpTravelSeg != nullptr &&
                  (tsw2->origMatch() || tsw2->destMatch())))
    {
      noPnrTrx->updateOpenDateIfNeccesary(tmpTravelSeg, toDepDateTime);
    }
  }

  if (UNLIKELY(fromTravelDOW == 0))
    fromTravelDOW = 7;

  retVal = validateReturnDate(toDepDateTime, earliestReturnDate);

  // check if we can set Sofpass for SOL
  if (!farePath && retVal == FAIL && trx.getTrxType() == PricingTrx::IS_TRX)
  {
    ShoppingTrx* shoppingTrx = dynamic_cast<ShoppingTrx*>(&trx);
    if (shoppingTrx && shoppingTrx->isSumOfLocalsProcessingEnabled())
    {
      retVal = validateReturnDate(itin.getMaxDepartureDT(), earliestReturnDate);

      if (retVal == PASS)
        retVal = SOFTPASS;
    }
  }

  if ((retVal != PASS) && (retVal != SOFTPASS))
  {
    const FareUsage* currentFuPtr = &fareUsage;
    FareUsage* fu = const_cast<FareUsage*>(currentFuPtr);
    fu->setFailedFound();
  }

  formatDiagnostic(diag, travelSeg2, retVal);

  // no need retVal == PASS, we could still use this fare as command pricing
  // or so
  if (LIKELY(earliestReturnDate.isValid()))
  {
    notifyObservers(minStayRule,
                    earliestReturnDate,
                    itin.segmentOrder(travelSeg2),
                    travelSeg2->boardMultiCity());
  }

  return retVal;
}

Record3ReturnTypes
MinimumStayApplication::getGeoTravelSegs(RuleUtil::TravelSegWrapperVector& travelSegs,
                                         const int& geoTblItemNo,
                                         PricingTrx& trx,
                                         const VendorCode& vendor,
                                         const FareUsage* fareUsage,
                                         const FarePath* farePath,
                                         const Itin& itin,
                                         const PricingUnit& pricingUnit,
                                         const bool origCheck,
                                         const bool destCheck)
{
  LOG4CXX_INFO(logger, " Entered MinimumStayApplication::getGeoTravelSegs()");

  bool fltStopCheck = false;
  TSICode tsiReturn;
  LocKey loc1Return;
  LocKey loc2Return;

  bool validateGeoReturnValue = false;

  if (fallback::fixed::APO29538_StopoverMinStay())
  {
    validateGeoReturnValue =
        RuleUtil::validateGeoRuleItem(geoTblItemNo,
                                      vendor,
                                      RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                      false,
                                      false,
                                      false,
                                      trx,
                                      farePath,
                                      &itin,
                                      &pricingUnit, // pricing unit
                                      nullptr,
                                      TrxUtil::getTicketingDT(trx),
                                      travelSegs, // this will contain the results
                                      origCheck,
                                      destCheck,
                                      fltStopCheck,
                                      tsiReturn,
                                      loc1Return,
                                      loc2Return,
                                      Diagnostic306);
  }
  else
  {
    validateGeoReturnValue =
        RuleUtil::validateGeoRuleItem(geoTblItemNo,
                                      vendor,
                                      RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                      false,
                                      false,
                                      false,
                                      trx,
                                      farePath,
                                      &itin,
                                      &pricingUnit, // pricing unit
                                      nullptr,
                                      TrxUtil::getTicketingDT(trx),
                                      travelSegs, // this will contain the results
                                      origCheck,
                                      destCheck,
                                      fltStopCheck,
                                      tsiReturn,
                                      loc1Return,
                                      loc2Return,
                                      Diagnostic306,
                                      RuleUtil::LOGIC_AND,
                                      fareUsage);
  }

  if (!validateGeoReturnValue)
  {
    LOG4CXX_INFO(logger, " Leaving MinimumStayApplication::getGeoTravelSegs() - SKIP");
    return SKIP;
  }

  _tsiToGeo = tsiReturn;

  LOG4CXX_INFO(logger, "Number of travel segs meeting tsi spec: " << travelSegs.size());

  if (!loc1Return.loc().empty())
    LOG4CXX_INFO(logger, "Location 1 " << loc1Return.loc());

  if (UNLIKELY(!loc2Return.loc().empty()))
    LOG4CXX_INFO(logger, "Location 2 " << loc2Return.loc());

  return PASS;
}

bool
MinimumStayApplication::softpassFareComponentPhase(PricingTrx& trx,
                                                   const Itin& itin,
                                                   const FareMarket& fm,
                                                   const PaxTypeFare& ptf,
                                                   Diag306Collector* diag) const
{
  // FC level validation doesn't work correctly if FROM/TO TSIs don't match any
  // segment on the fare component. Since RTW consists of single fare component
  // softpass shouldn't impact performance at all and it ensures that all fares are
  // validated correctly (while PU level, "heuristic-free" validation).
  if (UNLIKELY(RtwUtil::isRtw(trx)))
  {
    printDiagData(diag, Diag306Type::SOFTPASS_RTW_REQUEST);
    return true;
  }

  if (fm.direction() != FMDirection::OUTBOUND)
  {
    printDiagData(diag, Diag306Type::SOFTPASS_FM_DIRECT_NOT_OUTBOUND);
    return true;
  }

  if (ptf.owrt() == ONE_WAY_MAY_BE_DOUBLED || ptf.owrt() == ONE_WAY_MAYNOT_BE_DOUBLED)
  {
    printDiagData(diag, Diag306Type::SOFTPASS_PAXTYPE_FARE_ONEWAY);
    return true;
  }

  if (UNLIKELY(dynamic_cast<NoPNRPricingTrx*>(&trx)))
  {
    ptf.fare()->warningMap().set(WarningMap::cat6_warning);
  }

  if (PricingTrx::ESV_TRX != trx.getTrxType() && isDomesticUSCAOrTransborder(itin, trx))
  {
    printDiagData(diag, Diag306Type::SOFTPASS_DOMESTIC_OR_TRANSBORDER_FM);
    return true;
  }

  return false;
}

Record3ReturnTypes
MinimumStayApplication::validateOriginDOW(const MinStayRestriction* minStayRule,
                                          const uint32_t& travelDOW,
                                          uint32_t& minStayDOW) const
{
  Record3ReturnTypes retval;
  uint32_t ruleDOW = 0;

  LOG4CXX_INFO(logger, " Entered MinimumStayApplication::validateOriginDOW()");

  if (minStayRule->originDow().empty())
  {
    LOG4CXX_INFO(logger, "Origin Day of week is empty Day of Week does not apply PASS");
    return PASS;
  }

  uint32_t dowSize = minStayRule->originDow().size();

  retval = FAIL;

  LOG4CXX_INFO(logger, "Travel dow = " << travelDOW);

  for (uint32_t i = 0; i < dowSize; ++i)
  {
    ruleDOW = minStayRule->originDow()[i] - '0';
    LOG4CXX_INFO(logger, "Rule dow = " << ruleDOW);

    if (UNLIKELY(ruleDOW < 1 || ruleDOW > 7))
    {
      break;
    }

    if (ruleDOW == travelDOW)
    {
      retval = PASS;
      break;
    }
  }

  LOG4CXX_INFO(logger, " Leaving MinimumStayApplication::validateOriginDOW()");

  minStayDOW = ruleDOW;

  return retval;
}

Record3ReturnTypes
MinimumStayApplication::validateReturnDate(const DateTime& travelSegDepDate,
                                           const DateTime& earliestReturnDate) const
{
    if (travelSegDepDate < earliestReturnDate)
      return FAIL;
    return PASS;
}

DateTime
MinimumStayApplication::determineEarliestReturnDate(const DateTime minStayPeriodReturnDate,
                                                    const MinStayRestriction* minStayRule,
                                                    const PeriodOfStay& minStayPeriod,
                                                    Diag306Collector* diag)
{
  DateTime earliestReturnDate = MinMaxRulesUtils::determineReturnDate(
      minStayPeriodReturnDate, minStayRule->minStayDate(), minStayRule->earlierLaterInd());

  displayMinStayRules(diag, *minStayRule, minStayPeriod);
  displayReturnDates(diag, earliestReturnDate);

  earliestReturnDate = determineEarliestReturnDateWithTime(
      earliestReturnDate, minStayRule->tod(), minStayRule->minStayUnit());
  displayDeterminedMinStayDate(diag, earliestReturnDate);
  return earliestReturnDate;
}

DateTime
MinimumStayApplication::determineEarliestReturnDateWithTime(const DateTime& earliestReturnDate,
                                                            const int timeOfDay,
                                                            const std::string& minStayUnit) const
{
  DateTime resultDate(
      earliestReturnDate.year(), earliestReturnDate.month(), earliestReturnDate.day());

  uint64_t earliestDepTotalMinutes = 0;

  if (UNLIKELY(minStayUnit == HOURS || minStayUnit == MINUTES))
    earliestDepTotalMinutes = earliestReturnDate.totalMinutes();

  if (timeOfDay > 0 && static_cast<uint64_t>(timeOfDay) > earliestDepTotalMinutes)
    earliestDepTotalMinutes = timeOfDay;

  resultDate.addMinutes(earliestDepTotalMinutes);
  return resultDate;
}

Record3ReturnTypes
MinimumStayApplication::validateReturnDateFareDisplay(const DateTime& travelSegDepDate,
                                                      const DateTime& earliestReturnDate) const
{
  DateTime departureDate(travelSegDepDate.year(), travelSegDepDate.month(), travelSegDepDate.day());
  DateTime minStayReturnDate(
      earliestReturnDate.year(), earliestReturnDate.month(), earliestReturnDate.day());

  if (departureDate < minStayReturnDate)
    return FAIL;
  return PASS;
}

Record3ReturnTypes
MinimumStayApplication::validate(PricingTrx& trx,
                                 FareDisplayTrx* fareDisplayTrx,
                                 const FareMarket& fareMarket,
                                 Itin& itin,
                                 const PaxTypeFare& paxTypeFare,
                                 const RuleItemInfo* rule)
{
  LOG4CXX_DEBUG(logger, "Faremarket direction = " << fareMarket.getDirectionAsString());

  const MinStayRestriction* minStayRule = dynamic_cast<const MinStayRestriction*>(rule);

  if (!minStayRule)
    return SKIP;

  Diag306Collector* diag = prepare306Diag(trx);

  // Check for Immediate return conditions
  //
  Record3ReturnTypes retVal = SKIP;

  retVal = validateUnavailableDataTag(minStayRule->unavailTag());

  if (retVal != PASS)
  {
    printDataUnavailableFailed(diag, retVal, minStayRule);
    return retVal;
  }

  //-------------------------------------------------
  // Added for SITA v1.1
  // Value "000" of minStay means No Minimum Stay
  //-------------------------------------------------
  if (minStayRule->minStay() == NO_MINIMUM_STAY)
  { // no minimum stay

    printDiagData(diag, Diag306Type::NO_MINIMUM_STAY);
    return PASS;
  }

  // Check if Request wants to accept fares
  // with MinStay restriction
  bool nonFlexFareValidation = nonFlexFareValidationNeeded(trx);
  if (nonFlexFareValidation || isValidationNeeded(RuleConst::MINIMUM_STAY_RULE, trx))
  {
    printDiagData(diag, Diag306Type::MIN_STAY_FAILED);
    updateStatus(RuleConst::MINIMUM_STAY_RULE, FAIL);
    if (nonFlexFareValidation || shouldReturn(RuleConst::MINIMUM_STAY_RULE))
      return FAIL;
  }

  if (fareMarket.direction() != FMDirection::OUTBOUND)
  {
    printDiagData(diag, Diag306Type::SOFTPASS_FM_DIRECT_NOT_OUTBOUND);
    return SOFTPASS;
  }

  DateTime& returnDate = fareDisplayTrx->getRequest()->returnDate();

  if (paxTypeFare.owrt() == ONE_WAY_MAYNOT_BE_DOUBLED)
  {
    LOG4CXX_DEBUG(logger, "Paxtype fare is ONEWAY");

    if (!returnDate.isEmptyDate() && !returnDate.isValid())
    {
      printDiagData(diag, Diag306Type::FAIL_RETURN_DATE);
      return FAIL;
    }

    printDiagData(diag, Diag306Type::SOFTPASS_PAXTYPE_FARE_ONEWAY);
    return SOFTPASS;
  }

  if ((itin.travelSeg().size() == size_t(itin.segmentOrder(fareMarket.travelSeg().back()))) &&
      (fareMarket.travelSeg().size() == 1))
  {
    printDiagData(diag, Diag306Type::FM_END_IS_LAST_TVL_SEG);
    return PASS;
  }

  // We only want to validate the origin DOW if there
  // is one travel segment
  //
  if (!minStayRule->originDow().empty() && fareMarket.travelSeg().size() > 1)
  {
    printDiagData(diag, Diag306Type::SOFTPASS_MORE_THAN_ONE_TVL_SEG);
    return SOFTPASS;
  }
  TravelSeg* frontTrSeg = fareMarket.travelSeg().front();

  const PeriodOfStay minStayPeriod(minStayRule->minStay(), minStayRule->minStayUnit());

  const DateTime minStayPeriodReturnDate =
      RuleUtil::addPeriodToDate(frontTrSeg->departureDT(), minStayPeriod);

  const DateTime earliestReturnDate =
      determineEarliestReturnDate(minStayPeriodReturnDate, minStayRule, minStayPeriod, diag);

  DateTime startTravelDateTime = fareMarket.travelSeg()[0]->earliestDepartureDT();
  DateTime endTravelDateTime = fareMarket.travelSeg()[0]->latestDepartureDT();

  uint32_t ruleDOW = 0;
  retVal = validateOriginDOW(minStayRule, startTravelDateTime, endTravelDateTime, ruleDOW);

  if (retVal == FAIL)
  {
    printDiagData(diag, Diag306Type::ORIGIN_DAY_OF_WEEK_FAIL);
    return SKIP;
  }

  LOG4CXX_DEBUG(logger, "Validating Earliest Departure Date");
  LOG4CXX_DEBUG(logger,
                "Calculated Earliest Departure Date " << earliestReturnDate.toSimpleString());
  LOG4CXX_DEBUG(logger, "MinStayDate from Rule " << minStayRule->minStayDate().toSimpleString());
  LOG4CXX_DEBUG(logger, "Travel Seg Return Dep Date " << returnDate.toSimpleString());
  LOG4CXX_DEBUG(logger, "Travel Seg Return Arr Date " << returnDate.toSimpleString());
  LOG4CXX_DEBUG(logger, "Orig. Departure Date " << startTravelDateTime.toSimpleString());

  retVal = validateReturnDateFareDisplay(returnDate, earliestReturnDate);

  formatDiagnostic(diag, fareMarket.travelSeg().front(), retVal);
  return retVal;
}

Record3ReturnTypes
MinimumStayApplication::validateOriginDOW(const MinStayRestriction* minStayRule,
                                          DateTime& startTravelDate,
                                          DateTime& endTravelDate,
                                          uint32_t& minStayDOW) const
{
  Record3ReturnTypes retval;
  uint32_t ruleDOW = 0;

  if (minStayRule->originDow().empty())
  {
    LOG4CXX_INFO(logger, "Origin Day of week is empty Day of Week does not apply PASS");
    return PASS;
  }

  uint32_t dowSize = minStayRule->originDow().size();
  uint32_t travelDOW = 0;

  DateTime& travelDate = startTravelDate;

  retval = FAIL;

  for (uint32_t j = 0; j < 7 && travelDate <= endTravelDate; ++j, travelDate.nextDay())
  {
    travelDOW = travelDate.date().day_of_week().as_number();
    LOG4CXX_INFO(logger, "Travel dow = " << travelDOW);
    if (travelDOW == 0)
      travelDOW = 7;

    for (uint32_t i = 0; i < dowSize; ++i)
    {
      ruleDOW = minStayRule->originDow()[i] - '0';
      LOG4CXX_INFO(logger, "Rule dow = " << ruleDOW);

      if (ruleDOW < 1 || ruleDOW > 7)
      {
        minStayDOW = ruleDOW;
        return retval;
      }

      if (ruleDOW == travelDOW)
      {
        retval = PASS;
        break;
      }
    }
    if (retval == PASS)
      break;
  }
  minStayDOW = ruleDOW;

  return retval;
}
} // tse
