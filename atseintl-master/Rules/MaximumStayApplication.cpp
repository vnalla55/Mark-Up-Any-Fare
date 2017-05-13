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

#include "Rules/MaximumStayApplication.h"

#include "Common/DateTime.h"
#include "Common/FallbackUtil.h"
#include "Common/FareDisplayUtil.h"
#include "Common/Global.h"
#include "Common/ItinUtil.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/RtwUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseEnums.h"
#include "Common/TseUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Fare.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"
#include "DBAccess/LocKey.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagManager.h"
#include "Rules/IObserver.h"
#include "Rules/MinMaxRulesUtils.h"
#include "Rules/PeriodOfStay.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtil.h"
#include "Rules/RuleValidationChancelor.h"
#include "Util/BranchPrediction.h"

#include <exception>

namespace tse
{
FALLBACK_DECL(cat7DiagChange)

namespace
{
constexpr int LAST_MINUTE_OF_DAY = 1439; // 23:59
const std::string HOURS = "H";
const std::string MINUTES = "N";
constexpr char TICKET_ISSUE_IND = 'X';
constexpr char EARLIER = 'E';
constexpr char LATER = 'L';

Logger
logger("atseintl.Rules.MaximumStayApplication");

void
displayReturnDates(DiagManager& dc, const DateTime& maxStayPeriodReturnDate)
{
  LOG4CXX_INFO(logger, " Entered MaximumStayApplication::displayReturnDates()");

  dc << "CALCULATED MAXIMUM STAY DATES\n";

  dc << "RETURN DATE  - "
     << " ";

  if (maxStayPeriodReturnDate.isValid())
    dc << maxStayPeriodReturnDate.dateToSqlString().c_str();
  else
    dc << "N/A";

  dc << "\n";

  dc << "RETURN TIME  -  ";

  if (maxStayPeriodReturnDate.isValid())
  {
    std::string timeStr = maxStayPeriodReturnDate.timeToString(HHMM, ":");
    dc << std::setw(5) << timeStr;
  }
  else
    dc << "N/A";

  dc << "\n";

  LOG4CXX_INFO(logger, " Leaving MaximumStayApplication::displayReturnDates()");
}

void
displayDeterminedLatestDate(const DateTime& latestReturnDate, DiagManager& diag)
{
  diag << "VALIDATE AGAINST   -  ";

  diag << latestReturnDate.dateToString(YYYYMMDD, "-") << " ";
  diag << latestReturnDate.timeToString(HHMM, ":") << "\n";
}

void
displayMaxStayRules(DiagManager& diag,
                    const MaxStayRestriction* maxStayRule,
                    const PeriodOfStay& maxStayPeriod)
{
  LOG4CXX_INFO(logger, " Entered MaximumStayApplication::displayMaxStayRules()");

  if (maxStayPeriod.isValid())
  {
    if (maxStayPeriod.isDayOfWeek())
    {
      diag << "MAXSTAY - " << maxStayPeriod.unit() << " " << (std::string)maxStayPeriod;
    }
    else
    {
      diag << "MAXSTAY - " << (int)maxStayPeriod << " ";

      if (maxStayPeriod.unit() == PeriodOfStay::DAYS)
        diag << "DAY ";
      else if (maxStayPeriod.unit() == PeriodOfStay::MINUTES)
        diag << "MINUTE ";
      else if (maxStayPeriod.unit() == PeriodOfStay::HOURS)
        diag << "HOUR";
      else if (maxStayPeriod.unit() == PeriodOfStay::MONTHS)
        diag << "MONTH ";
    }
  }
  else
    diag << "MAXSTAY - N/A";

  diag << "\n";

  diag << "RETURN TRAVEL IND: ";

  if (maxStayRule->returnTrvInd() == RuleConst::RETURN_TRV_COMMENCE_IND)
    diag << "\n"
         << "DEPARTURE MUST COMMENCE NO LATER THAN MAX STAY";
  else if (maxStayRule->returnTrvInd() == RuleConst::RETURN_TRV_COMPLETE_IND)
    diag << "\n"
         << "ARRIVAL MUST BE COMPLETED NO LATER THAN MAX STAY";
  else
    diag << "N/A";

  diag << "\n";

  diag << "TICKET ISSUE IND: ";

  if (maxStayRule->tktIssueInd() == TICKET_ISSUE_IND)
    diag << maxStayRule->tktIssueInd();
  else
    diag << "N/A";

  diag << "\n";

  if (!maxStayRule->maxStayDate().isValid())
    diag << "MAXSTAY DATE - N/A ";
  else
  {
    diag << "MAXSTAY DATE - ";

    diag << maxStayRule->maxStayDate().dateToSqlString();

    diag << " ";
  }

  diag << "FROM GEO - " << maxStayRule->geoTblItemNoFrom() << "  "
       << "TO GEO - " << maxStayRule->geoTblItemNoTo();

  diag << "\n";

  diag << "TABLE 994 - " << maxStayRule->overrideDateTblItemNo() << "    ";

  if ((maxStayRule->earlierLaterInd() == EARLIER) || (maxStayRule->earlierLaterInd() == LATER))
    diag << "EARLIER/LATER IND -  " << maxStayRule->earlierLaterInd();
  else
    diag << "EARLIER/LATER IND - N/A";

  if (maxStayRule->tod() > 0)
    diag << " TIME -  " << maxStayRule->tod();
  else
    diag << " TIME -  N/A"
         << "\n";

  diag << "\n";

  LOG4CXX_INFO(logger, " Leaving MaximumStayApplication::displayMaxStayRules()");
}

void
displayReturnDateAndMaybeMaxStayRules(DiagManager& diag,
                                      const MaxStayRestriction* maxStayRule,
                                      const PeriodOfStay& maxStayPeriod,
                                      const DateTime& latestReturnDate,
                                      bool showFullDiag)

{
  if (showFullDiag)
  {
    diag << "CATEGORY 7 RULE DATA\n";
    displayMaxStayRules(diag, maxStayRule, maxStayPeriod);
  }
  displayReturnDates(diag, latestReturnDate);
}

}

bool
MaximumStayApplication::nonFlexFareValidationNeeded(PricingTrx& trx) const
{
  return (!trx.isFlexFare() && trx.getOptions()->isNoMinMaxStayRestr());
}

template <typename T>
void
MaximumStayApplication::printDiag(T& diag, std::string msg)
{
  if (UNLIKELY(diag.isActive()))
    diag << msg << "\n";
}

template <typename T>
Record3ReturnTypes
MaximumStayApplication::validateMaximumStayRule(const MaxStayRestriction& maxStayRule, T& diag)
{
  Record3ReturnTypes result = PASS;
  PeriodOfStay maxStayPeriod(maxStayRule.maxStay(), maxStayRule.maxStayUnit());

  if (!maxStayPeriod.isOneYear())
  {
    LOG4CXX_INFO(logger, " Leaving MaxStayApplication::validate() - FAIL");
    printDiag(diag, " MAXSTAY: FAILED - RESTRICTION APPLY");
    result = FAIL;
  }
  printDiag(diag, " MAXSTAY: 12M - NO RESTRICTION APPLY");
  return result;
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
MaximumStayApplication::validate(PricingTrx& trx,
                                 Itin& itin,
                                 const PaxTypeFare& paxTypeFare,
                                 const RuleItemInfo* rule,
                                 const FareMarket& fareMarket)
{
  LOG4CXX_INFO(logger, " Entered MaximumStayApplication::validate()");

  int failCount = 0;
  int passCount = 0;

  NoPNRPricingTrx* noPnrTrx = dynamic_cast<NoPNRPricingTrx*>(&trx);
  const MaxStayRestriction* maxStayRule = dynamic_cast<const MaxStayRestriction*>(rule);

  if (UNLIKELY(!maxStayRule))
    return SKIP;

  DiagManager diag(trx, Diagnostic307);
  Record3ReturnTypes retVal = validateUnavailableDataTag(maxStayRule->unavailTag());

  if (retVal != PASS)
  {
    if (UNLIKELY(diag.isActive()))
    {
      if (retVal == SKIP)
        diag << " MAXSTAY: SKIPPED";
      else
        diag << " MAXSTAY: FAILED";

      diag << "- UNAVAILABLE DATA TAG : " << maxStayRule->unavailTag() << "\n";
    }

    return retVal;
  }

  bool nonFlexFareValidation = nonFlexFareValidationNeeded(trx);
  if (nonFlexFareValidation || isValidationNeeded(RuleConst::MAXIMUM_STAY_RULE, trx))
  {
    Record3ReturnTypes result = validateMaximumStayRule(*maxStayRule, diag);
    updateStatus(RuleConst::MAXIMUM_STAY_RULE, result);
    if (nonFlexFareValidation || shouldReturn(RuleConst::MAXIMUM_STAY_RULE))
      return result;
  }

  if (softpassFareComponentValidation(trx, itin, fareMarket, paxTypeFare, diag))
    return SOFTPASS;

  std::vector<TravelSeg*> applTravelSegment;
  TravelSeg* frontTrSeg = fareMarket.travelSeg().front();
  DateTime travelDate = frontTrSeg->departureDT();

  if (UNLIKELY(noPnrTrx != nullptr))
    noPnrTrx->updateOpenDateIfNeccesary(frontTrSeg, travelDate);

  if ((itin.travelSeg().size() == size_t(itin.segmentOrder(fareMarket.travelSeg().back()))) &&
      (fareMarket.travelSeg().size() == 1))
    return PASS;

  if (UNLIKELY(noPnrTrx != nullptr && itin.dateType() == Itin::NoDate))
  {
    LOG4CXX_INFO(logger, "AUTOPASS OPEN SEGMENT - FOR WQ TRX [1]");
    return PASS;
  }

  if (UNLIKELY(frontTrSeg != nullptr && noPnrTrx == nullptr && frontTrSeg->isOpenWithoutDate() &&
      ItinUtil::isOpenSegAfterDatedSeg(itin, frontTrSeg)))
  {
    if (UNLIKELY(diag.isActive()))
      diag << "MAXSTAY PASS: \n"
           << "OPEN SEGMENT WITHOUT DATE \n";

    LOG4CXX_INFO(logger, " Leaving MaximumStayApplication::validate() - PASS OPEN SEGMENT");
    return SOFTPASS;
  }

  if (LIKELY(!itin.travelSeg().empty()))
  {
    for (size_t i = size_t(itin.segmentOrder(fareMarket.travelSeg().front()));
         i < itin.travelSeg().size();
         i++)
    {
      TravelSeg* travelSeg = itin.travelSeg()[i];
      applTravelSegment.push_back(travelSeg);
    }
  }
  else
  {
    LOG4CXX_INFO(logger, " Leaving MaximumStayApplication::validate() - FAIL");

    if (UNLIKELY(diag.isActive()))
      diag << " MAXSTAY: FAILED "
           << "- ITIN TRAVEL SEG EMPTY\n";

    return FAIL;
  }

  DateTime latestReturnDate = determineLatestReturnDate(maxStayRule, travelDate, diag);

  std::vector<TravelSeg*>::reverse_iterator applTravelSegmentI = applTravelSegment.rbegin();
  std::vector<TravelSeg*>::reverse_iterator travelSegEnd = applTravelSegment.rend();

  for (; applTravelSegmentI != travelSegEnd; applTravelSegmentI++)
  {
    DateTime travelSegDepDate = (*applTravelSegmentI)->departureDT();
    if (UNLIKELY(noPnrTrx != nullptr))
      noPnrTrx->updateOpenDateIfNeccesary(*applTravelSegmentI, travelSegDepDate);

    if (UNLIKELY((*applTravelSegmentI)->isOpenWithoutDate() && noPnrTrx == nullptr &&
        ItinUtil::isOpenSegAfterDatedSeg(itin, *applTravelSegmentI)))
    {
      if (UNLIKELY(diag.isActive()))
        diag << "MAXSTAY PASS: \n"
             << "OPEN SEGMENT WITHOUT DATE \n";

      LOG4CXX_INFO(logger, " Leaving MaximumStayApplication::validate() - PASS OPEN SEGMENT");
      retVal = PASS;
    }
    else
    {
      retVal = validateReturnDate(travelSegDepDate, latestReturnDate);
    }

    if (UNLIKELY(diag.isActive()))
      formatDiagnostic(diag.collector(), *applTravelSegmentI, retVal);

    if (retVal == PASS)
      passCount++;
    else if (retVal == FAIL)
      failCount++;
  }

  if ((failCount > 0) && (passCount > 0 || noPnrTrx == nullptr))
  {
    if (UNLIKELY(diag.isActive()))
    {
      diag << "\n";
      diag << "MAXSTAY SOFTPASS: NEED REVALIDATION FOR PRICING UNIT\n";
    }
    retVal = SOFTPASS;
  }

  return retVal;
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
MaximumStayApplication::validate(PricingTrx& trx,
                                 const RuleItemInfo* rule,
                                 const FarePath& farePath,
                                 const PricingUnit& pricingUnit,
                                 const FareUsage& fareUsage)
{

  return validate(trx, rule, &farePath, *farePath.itin(), pricingUnit, fareUsage);
}

Record3ReturnTypes
MaximumStayApplication::validate(PricingTrx& trx,
                                 const RuleItemInfo* rule,
                                 const FarePath* farePath,
                                 const Itin& itin,
                                 const PricingUnit& pricingUnit,
                                 const FareUsage& fareUsage)
{
  LOG4CXX_INFO(logger, " Entered MaximumStayApplication::validate() for PricingUnit");
  Record3ReturnTypes retVal = SKIP;

  const MaxStayRestriction* maxStayRule = dynamic_cast<const MaxStayRestriction*>(rule);

  if (UNLIKELY(!maxStayRule))
    return retVal;

  NoPNRPricingTrx* noPnrTrx = dynamic_cast<NoPNRPricingTrx*>(&trx);

  DiagManager diag(trx, Diagnostic307);
  retVal = validateUnavailableDataTag(maxStayRule->unavailTag());

  if (retVal != PASS)
  {
    if (UNLIKELY(diag.isActive()))
    {
      if (retVal == SKIP)
        diag << " MAXSTAY: SKIPPED";
      else
        diag << " MAXSTAY: FAILED";

      diag << "- UNAVAILABLE DATA TAG : " << maxStayRule->unavailTag() << "\n";
    }

    return retVal;
  }

  bool nonFlexFareValidation = nonFlexFareValidationNeeded(trx);
  if (nonFlexFareValidation || isValidationNeeded(RuleConst::MAXIMUM_STAY_RULE, trx))
  {
    Record3ReturnTypes result = validateMaximumStayRule(*maxStayRule, diag);
    updateStatus(RuleConst::MAXIMUM_STAY_RULE, result);
    if (nonFlexFareValidation || shouldReturn(RuleConst::MAXIMUM_STAY_RULE))
      return result;
  }

  if (pricingUnit.puType() == PricingUnit::Type::ONEWAY)
  {
    LOG4CXX_INFO(logger, " Leaving MaximumStayApplication::validate() - PASS");

    if (UNLIKELY(diag.isActive()))
    {
      diag << "MAXSTAY PASS: \n";
      diag << "PRICING UNIT IS ONEWAY \n";
    }

    return PASS;
  }

  if (UNLIKELY(noPnrTrx != nullptr))
    fareUsage.paxTypeFare()->warningMap().set(WarningMap::cat7_warning);

  PeriodOfStay maxStayPeriod(maxStayRule->maxStay(), maxStayRule->maxStayUnit());

  if (UNLIKELY(!(maxStayPeriod.isValid()) && (maxStayRule->maxStayDate().isEmptyDate())))
  {
    if (UNLIKELY(diag.isActive()))
      diag << " MAXSTAY SKIPPED: NO MAX STAY RESTRICTION \n";

    return SKIP;
  }

  DateTime ticketingDate = TrxUtil::getTicketingDT(trx);
  RuleUtil::TravelSegWrapperVector applTravelSegment;
  const bool useTicketingDate = (maxStayRule->tktIssueInd() == TICKET_ISSUE_IND);

  if (!useTicketingDate)
  {
    if (UNLIKELY((maxStayRule->geoTblItemNoFrom() == 0) || (maxStayRule->geoTblItemNoTo() == 0)))
    {
      if (UNLIKELY(diag.isActive()))
      {
        diag << "MAXSTAY FAILED: ";

        if (maxStayRule->geoTblItemNoFrom() == 0)
          diag << "MAXSTAY GEO TBL ITEM NO FROM IS ZERO \n";

        if (maxStayRule->geoTblItemNoTo() == 0)
          diag << "MAXSTAY GEO TBL ITEM NO TO IS ZERO \n";
      }
      return FAIL;
    }
  }
  else
  {
    if ((maxStayRule->geoTblItemNoTo() == 0))
    {
      if (UNLIKELY(diag.isActive()))
      {
        diag << "MAXSTAY FAILED: ";

        if (maxStayRule->geoTblItemNoTo() == 0)
          diag << "MAXSTAY GEO TBL ITEM NO TO IS ZERO AND TICKET ISSUE IND. IS X \n";
      }
      return FAIL;
    }
  }

  if (fallback::cat7DiagChange(&trx))
  {
    if (UNLIKELY(diag.isActive()))
    {
      diag << "CATEGORY 7 RULE DATA\n";
      displayMaxStayRules(diag, maxStayRule, maxStayPeriod);
    }
  }

  DateTime fromDepDateTime = trx.transactionStartTime();
  TravelSeg* fromGeoTravelSeg = nullptr;
  bool noPnrWithOutDate = (noPnrTrx != nullptr && itin.dateType() == Itin::NoDate);

  if (!useTicketingDate)
  {
    retVal =
        getGeoTravelSegs(applTravelSegment,
                         maxStayRule->geoTblItemNoFrom(),
                         trx,
                         diag.isActive() ? diag.collector() : *DCFactory::instance()->create(trx),
                         fareUsage,
                         farePath,
                         itin,
                         pricingUnit,
                         true,
                         true);

    if (retVal != PASS)
      return retVal;

    const RuleUtil::TravelSegWrapper* tsw1 = applTravelSegment.front();
    fromGeoTravelSeg = tsw1->travelSeg();

    if (fromGeoTravelSeg->isOpenWithoutDate() && noPnrTrx == nullptr &&
        ItinUtil::isOpenSegAfterDatedSeg(itin, fromGeoTravelSeg))
    {
      if (UNLIKELY(diag.isActive()))
        diag << "MAXSTAY PASS: \n"
             << "OPEN SEGMENT WITHOUT DATE \n";

      LOG4CXX_INFO(logger, " Leaving MaximumStayApplication::validate() - PASS OPEN SEGMENT");
      return PASS;
    }

    if (tsw1->origMatch())
      fromDepDateTime = fromGeoTravelSeg->departureDT();
    else if (tsw1->destMatch())
      fromDepDateTime = fromGeoTravelSeg->arrivalDT();

    if (UNLIKELY(noPnrTrx != nullptr && itin.dateType() != Itin::NoDate))
      noPnrTrx->updateOpenDateIfNeccesary(fromGeoTravelSeg, fromDepDateTime);
  }

  retVal =
      getGeoTravelSegs(applTravelSegment,
                       maxStayRule->geoTblItemNoTo(),
                       trx,
                       diag.isActive() ? diag.collector() : *DCFactory::instance()->create(trx),
                       fareUsage,
                       farePath,
                       itin,
                       pricingUnit,
                       true,
                       true);

  if (retVal != PASS)
  {
    if (noPnrWithOutDate)
      return PASS;
    else
      return retVal;
  }

  const bool isRtw = trx.getOptions() && trx.getOptions()->isRtw();

  if (LIKELY(!isRtw))
  {
    if (removeGeoTravelSegs(applTravelSegment, fareUsage, pricingUnit, getTsiTo(),&trx) &&
        applTravelSegment.empty())
    {
      if (UNLIKELY(diag.isActive() && !noPnrWithOutDate))
      {
        diag << "MAXSTAY SKIPPED:\n";
        diag << "GEO TRAVEL SEGMENTS CONTAINED IN FARE USAGE\n";
      }

      if (noPnrWithOutDate)
        return PASS;
      else
        return SKIP;
    }
  }

  DateTime& maxStayPeriodReturnDate = useTicketingDate ? ticketingDate : fromDepDateTime;

  DateTime latestReturnDate = determineLatestReturnDate(
      maxStayRule, maxStayPeriodReturnDate, diag, !fallback::cat7DiagChange(&trx));

  DateTime tsiDateTime = trx.transactionStartTime();
  const RuleUtil::TravelSegWrapper* tsw2 = applTravelSegment.back();
  const TravelSeg* toGeoTravelSeg = tsw2->travelSeg();

  if (UNLIKELY(noPnrWithOutDate))
  {
    notifyObservers(maxStayRule->returnTrvInd(),
                    maxStayRule,
                    latestReturnDate,
                    itin.segmentOrder(toGeoTravelSeg),
                    toGeoTravelSeg->boardMultiCity());

    LOG4CXX_INFO(logger, " AUTOPASS OPEN SEGMENT - FOR WQ TRX [2]");
    return PASS;
  }

  if (toGeoTravelSeg->isOpenWithoutDate() && noPnrTrx == nullptr &&
      ItinUtil::isOpenSegAfterDatedSeg(itin, toGeoTravelSeg))
  {
    if (UNLIKELY(diag.isActive()))
      diag << "MAXSTAY PASS: \n"
           << "OPEN SEGMENT WITHOUT DATE \n";

    LOG4CXX_INFO(logger, " Leaving MaximumStayApplication::validate() - PASS OPEN SEGMENT");

    notifyObservers(maxStayRule->returnTrvInd(),
                    maxStayRule,
                    latestReturnDate,
                    itin.segmentOrder(toGeoTravelSeg),
                    toGeoTravelSeg->boardMultiCity());
    return PASS;
  }

  {
    const TravelSeg* tmpTravelSeg = toGeoTravelSeg;
    if (tsw2->origMatch())
    {
      tsiDateTime = toGeoTravelSeg->departureDT();
    }
    else if (tsw2->destMatch())
    {
      if (!useTicketingDate)
      {
        if (fromGeoTravelSeg != toGeoTravelSeg)
          tsiDateTime = toGeoTravelSeg->arrivalDT();
        else
        {
          const TravelSeg* lastPuTravelSeg = pricingUnit.travelSeg().back();
          tmpTravelSeg = lastPuTravelSeg;
          if (lastPuTravelSeg)
            tsiDateTime = lastPuTravelSeg->departureDT();
        }
      }
      else
        tsiDateTime = toGeoTravelSeg->arrivalDT();
    }
    if (UNLIKELY(noPnrTrx != nullptr && tmpTravelSeg != nullptr && (tsw2->origMatch() || tsw2->destMatch())))
      noPnrTrx->updateOpenDateIfNeccesary(tmpTravelSeg, tsiDateTime);
  }

  retVal = validateReturnDate(tsiDateTime, latestReturnDate);

  if (UNLIKELY(diag.isActive()))
    formatDiagnostic(diag.collector(), toGeoTravelSeg, retVal, maxStayRule->returnTrvInd());

  if (LIKELY(latestReturnDate.isValid()))
  {
      notifyObservers(maxStayRule->returnTrvInd(),
                      maxStayRule,
                      latestReturnDate,
                      itin.segmentOrder(toGeoTravelSeg),
                      toGeoTravelSeg->boardMultiCity());

  }

  return retVal;
}

DateTime
MaximumStayApplication::determineLatestReturnDateWithTime(const DateTime& latestReturnDate,
                                                          const int timeOfDay,
                                                          const std::string& maxStayUnit) const
{
  DateTime resultDate(latestReturnDate.year(), latestReturnDate.month(), latestReturnDate.day());

  uint64_t latestDepTotalMinutes = LAST_MINUTE_OF_DAY;

  if (UNLIKELY(maxStayUnit == HOURS || maxStayUnit == MINUTES))
    latestDepTotalMinutes = latestReturnDate.totalMinutes();

  if (timeOfDay > 0 && static_cast<uint64_t>(timeOfDay) < latestDepTotalMinutes)
    latestDepTotalMinutes = timeOfDay;

  resultDate.addMinutes(latestDepTotalMinutes);

  return resultDate;
}

DateTime
MaximumStayApplication::determineLatestReturnDate(const MaxStayRestriction* maxStayRule,
                                                  const DateTime& dateToValidate,
                                                  DiagManager& diag,
                                                  bool showFullDiag) const
{
  TSE_ASSERT(maxStayRule);
  const PeriodOfStay maxStayPeriod(maxStayRule->maxStay(), maxStayRule->maxStayUnit());
  const DateTime maxStayPeriodReturnDate = RuleUtil::addPeriodToDate(dateToValidate, maxStayPeriod);

  DateTime latestReturnDate = MinMaxRulesUtils::determineReturnDate(
      maxStayPeriodReturnDate, maxStayRule->maxStayDate(), maxStayRule->earlierLaterInd());

  if (UNLIKELY(diag.isActive()))
    displayReturnDateAndMaybeMaxStayRules(
        diag, maxStayRule, maxStayPeriod, latestReturnDate, showFullDiag);

  latestReturnDate = determineLatestReturnDateWithTime(
      latestReturnDate, maxStayRule->tod(), maxStayRule->maxStayUnit());

  if (UNLIKELY(diag.isActive()))
    displayDeterminedLatestDate(latestReturnDate, diag);
  return latestReturnDate;
}
//-------------------------------------------------------------------
//   @method validate
//
//   Description: Performs rule validations on a PricingUnit.
//
//   @param PricingTrx           - Pricing transaction
//   @param RuleItemInfo         - Record 2 Rule Item Segment Info
//   @param Itin                 - Itin
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
MaximumStayApplication::validate(PricingTrx& trx,
                                 const RuleItemInfo* rule,
                                 const Itin& itin,
                                 const PricingUnit& pricingUnit,
                                 const FareUsage& fareUsage)
{
  return validate(trx, rule, nullptr, itin, pricingUnit, fareUsage);
}

bool
MaximumStayApplication::softpassFareComponentValidation(PricingTrx& trx,
                                                        const Itin& itin,
                                                        const FareMarket& fm,
                                                        const PaxTypeFare& ptf,
                                                        DiagManager& dc) const
{
  // FC level validation doesn't work correctly if FROM/TO TSIs don't match any
  // segment on the fare component. Since RTW consists of single fare component
  // softpass shouldn't impact performance at all and it ensures that all fares are
  // validated correctly (while PU level, "heuristic-free" validation).
  if (UNLIKELY(RtwUtil::isRtw(trx)))
  {
    if (UNLIKELY(dc.isActive()))
      dc << "MAXSTAY SOFTPASS: RTW REQUEST\n";
    return true;
  }

  if (fm.direction() != FMDirection::OUTBOUND)
  {
    if (UNLIKELY(dc.isActive()))
    {
      dc << "MAXSTAY SOFTPASS -\n";
      dc << "FAREMARKET DIRECTIONALITY NOT OUTBOUND - NEED REVALIDATION\n";
    }
    return true;
  }

  if (ptf.owrt() == ONE_WAY_MAY_BE_DOUBLED || ptf.owrt() == ONE_WAY_MAYNOT_BE_DOUBLED)
  {
    if (UNLIKELY(dc.isActive()))
      dc << "MAXSTAY SOFTPASS: \nPAXTYPE FARE IS ONEWAY\n";
    return true;
  }

  if (UNLIKELY(dynamic_cast<NoPNRPricingTrx*>(&trx)))
    ptf.fare()->warningMap().set(WarningMap::cat7_warning);

  if (PricingTrx::ESV_TRX != trx.getTrxType() && isDomesticUSCAOrTransborder(itin, trx))
  {
    if (UNLIKELY(dc.isActive()))
      dc << "MAXSTAY SOFTPASS: DOMESTIC OR TRANSBORDER FARE MARKET\n";
    return true;
  }

  return false;
}

//-------------------------------------------------------------------
//   @method formatDiagnostic
//
//   Description: Writes the detail information to the diagnostic
//
//   @param  DiagCollector       - diagnostic collector
//   @param  TravelSeg           - travel segment
//
//   @return void
//
//-------------------------------------------------------------------
void
MaximumStayApplication::formatDiagnostic(DiagCollector& diag,
                                         const TravelSeg* travelSeg,
                                         Record3ReturnTypes retval,
                                         const Indicator arrDepInd)
{
  LOG4CXX_INFO(logger, " Entered MaximumStayApplication::formatDiagnostic()");

  DateTime tvlSegDateTime = DateTime::localTime();

  const Loc& orig = *(travelSeg)->origin();
  const Loc& dest = *(travelSeg)->destination();

  if (arrDepInd == RuleConst::RETURN_TRV_COMMENCE_IND)
  {
    tvlSegDateTime = travelSeg->departureDT();
    diag << "ITINERARY DEPARTURE DATE AND TIMES\n";
    diag << "DEPARTURE      - " << orig.loc().c_str() << " ";
  }
  else
  {
    diag << "ITINERARY ARRIVAL DATE AND TIMES\n";
    diag << "ARRIVAL        - " << dest.loc().c_str() << " ";
    tvlSegDateTime = travelSeg->arrivalDT();
  }

  std::string retStr;

  if (retval == PASS)
    retStr = "PASS";
  else if (retval == FAIL)
    retStr = "FAIL";
  else if (retval == SKIP)
    retStr = "SKIP";

  diag << tvlSegDateTime.dateToSqlString().c_str() << " ";

  std::string timeStr = tvlSegDateTime.timeToString(HHMM, ":");
  diag << timeStr;

  diag << " " << retStr.c_str() << "\n";

  diag << "DAY OF WEEK    - " << WEEKDAYS_UPPER_CASE[tvlSegDateTime.dayOfWeek()] << "\n";

  LOG4CXX_INFO(logger, " Leaving MaximumStayApplication::formatDiagnostic()");
}

//----------------------------------------------------------------------------------------
//   @method validateReturnDate
//
//   Description: Validates the return date based on the minimum stay period and/or
//                the minimum stay date.
//
//   @param  DateTime            - Travel Seg departure date, using last point of
//                                 departure in travel seg vector
//   @param  DateTime            - calculated earliest date of departure based on minimum
//                                 stay period rules
//   @param  int                 - time of day
//
//   @return Record3ReturnTypes  - possible values are:
//                                 NOT_PROCESSED = 1
//                                 FAIL          = 2
//                                 PASS          = 3
//                                 SKIP          = 4
//                                 STOP          = 5
//----------------------------------------------------------------------------------------
Record3ReturnTypes
MaximumStayApplication::validateReturnDate(const DateTime& travelSegDepDate,
                                           const DateTime& maxStayPeriodReturnDate)

{
  return (travelSegDepDate > maxStayPeriodReturnDate) ? FAIL : PASS;
}

//-----------------------------------------------------------------
//   @method displayReturnDate
//
//   Description: Writes the calcuated return dates  to the
//                diagnostic.
//
//   @param  DiagCollector       - diagnostic
//   @param  DateTime            - earliest return date
//
//   @return void
//-----------------------------------------------------------------

Record3ReturnTypes
MaximumStayApplication::getGeoTravelSegs(RuleUtil::TravelSegWrapperVector& travelSegs,
                                         const int& geoTblItemNo,
                                         PricingTrx& trx,
                                         DiagCollector& diag,
                                         const FareUsage& fareUsage,
                                         const FarePath* farePath,
                                         const Itin& itin,
                                         const PricingUnit& pricingUnit,
                                         const bool origCheck,
                                         const bool destCheck)
{
  LOG4CXX_INFO(logger, " Entered MaximumStayApplication::getGeoTravelSegs()");

  RuleConst::TSIScopeParamType scopeParam = RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY;

  const DateTime tktDate = TrxUtil::getTicketingDT(trx);

  bool fltStopCheck = false;
  TSICode tsiReturn;
  LocKey loc1Return;
  LocKey loc2Return;

  if (UNLIKELY(!RuleUtil::validateGeoRuleItem(geoTblItemNo,
                                     fareUsage.paxTypeFare()->vendor(),
                                     scopeParam,
                                     false,
                                     false,
                                     false,
                                     trx,
                                     farePath,
                                     &itin,
                                     &pricingUnit,
                                     nullptr,
                                     tktDate,
                                     travelSegs,
                                     origCheck,
                                     destCheck,
                                     fltStopCheck,
                                     tsiReturn,
                                     loc1Return,
                                     loc2Return,
                                     Diagnostic307,
                                     RuleUtil::LOGIC_AND)))
  {
    LOG4CXX_INFO(logger, " Leaving MaximumStayApplication::validate() - SKIP");

    if (UNLIKELY(diag.diagnosticType() == Diagnostic307))
    {
      diag << " MAXSTAY: SKIPPED "
           << "- FALSE FROM VALIDATE GEO RULE ITEM\n";
    }

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
MaximumStayApplication::validate(PricingTrx& trx,
                                 FareDisplayTrx* fareDisplayTrx,
                                 const FareMarket& fareMarket,
                                 Itin& itin,
                                 const PaxTypeFare& paxTypeFare,
                                 const RuleItemInfo* rule)
{
  LOG4CXX_INFO(logger, " Entered MaximumStayApplication::validate()");

  const MaxStayRestriction* maxStayRule = dynamic_cast<const MaxStayRestriction*>(rule);

  if (!maxStayRule)
    return SKIP;

  DiagManager diag(trx, Diagnostic307);

  // Check for Immediate return conditions
  //
  Record3ReturnTypes retVal = validateUnavailableDataTag(maxStayRule->unavailTag());

  if (retVal != PASS)
  {
    if (UNLIKELY(diag.isActive()))
    {
      if (retVal == SKIP)
        diag << " MAXSTAY: SKIPPED";
      else
        diag << " MAXSTAY: FAILED";

      diag << "- UNAVAILABLE DATA TAG : " << maxStayRule->unavailTag() << "\n";
    }

    return retVal;
  }

  // Check if Request wants to accept fares
  // with MaxStay restriction
  const bool nonFlexFareValidation = nonFlexFareValidationNeeded(trx);
  if (nonFlexFareValidation || isValidationNeeded(RuleConst::MAXIMUM_STAY_RULE, trx))
  {
    const Record3ReturnTypes result = validateMaximumStayRule(*maxStayRule, diag);
    updateStatus(RuleConst::MAXIMUM_STAY_RULE, result);
    if (nonFlexFareValidation || shouldReturn(RuleConst::MAXIMUM_STAY_RULE))
      return result;
  }

  if (fareMarket.direction() != FMDirection::OUTBOUND)
  {
    if (UNLIKELY(diag.isActive()))
    {
      diag << "MAXSTAY SOFTPASS -\n";
      diag << "FAREMARKET DIRECTIONALITY NOT OUTBOUND - NEED REVALIDATION\n";
    }

    return SOFTPASS;
  }

  const DateTime& returnDate = fareDisplayTrx->getRequest()->returnDate();
  if (paxTypeFare.owrt() == ONE_WAY_MAYNOT_BE_DOUBLED)
  {
    if (UNLIKELY(diag.isActive()))
    {
      diag << "MAXSTAY SOFTPASS: \n";
      diag << "PAXTYPE FARE IS ONEWAY \n";
    }

    if (!returnDate.isEmptyDate() && !returnDate.isValid())
    {
      if (UNLIKELY(diag.isActive()))
      {
        diag << "MAXSTAY FAILED: "
             << "- EMPTY OR INVALID RETURN DATE \n";
      }
      return FAIL;
    }
    if (UNLIKELY(diag.isActive()))
    {
      diag << "MAXSTAY SOFTPASS: "
           << "PAXTYPE FARE IS ONEWAY\n";
    }
    return SOFTPASS;
  }



  if ((itin.travelSeg().size() == size_t(itin.segmentOrder(fareMarket.travelSeg().back()))) &&
      (fareMarket.travelSeg().size() == 1))
  {
    //      LOG4CXX_DEBUG(logger, "Faremarket ending segment is last travel seg in itin");
    if (UNLIKELY(diag.isActive()))
    {
      diag << "MAXSTAY PASSED\n";
    }
    return PASS;
  }

  const TravelSeg* frontTrSeg = fareMarket.travelSeg().front();
  const DateTime travelDate = frontTrSeg->latestDepartureDT();
  const DateTime latestReturnDate = determineLatestReturnDate(maxStayRule, travelDate, diag);
  retVal = validateReturnDate(returnDate, latestReturnDate);

  if (UNLIKELY(diag.isActive()))
  {
    formatDiagnostic(diag.collector(), fareMarket.travelSeg().front(), retVal);
  }
  return retVal;
}
} // tse
