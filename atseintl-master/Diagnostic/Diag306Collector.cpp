//----------------------------------------------------------------------------
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//----------------------------------------------------------------------------

#include "Diagnostic/Diag306Collector.h"

#include "Common/DateTime.h"
#include "Common/FallbackUtil.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/MinStayRestriction.h"
#include "Rules/MinimumStayApplication.h"
#include "Rules/MinMaxRulesUtils.h"
#include "Rules/PeriodOfStay.h"

namespace tse
{
FIXEDFALLBACK_DECL(APO29538_StopoverMinStay)

void
Diag306Collector::printDiagData(Diag306Type diag306Type, const MinStayRestriction* minStayRule)
{
  if (LIKELY(!_active))
    return;

  switch (diag306Type)
  {
  case Diag306Type::NO_MINIMUM_STAY:
    *this << "MINSTAY PASS: \n";
    *this << " NO MINIMUM STAY RESTRICTION\n";
    break;

  case Diag306Type::MIN_STAY_FAILED:
    *this << " MINSTAY: FAILED  - RESTRICTION APPLY\n";
    break;

  case Diag306Type::PASS_ONEWAY:
    *this << "MINSTAY PASS: \n"
          << "PRICING UNIT IS ONEWAY\n";
    break;

  case Diag306Type::SKIP_GEO_RULE:
    *this << " MINSTAY: SKIPPED "
          << "- FALSE FROM VALIDATE GEO RULE ITEM\n";
    break;

  case Diag306Type::PASS_OPEN_SEGMENT:
    *this << "MINSTAY PASS: \n"
          << "OPEN SEGMENT WITHOUT DATE \n";
    break;

  case Diag306Type::SAME_GEO_IN_FARE_USAGE:
    *this << "MINSTAY SKIPPED: \n";
    *this << "GEO TRAVEL SEGS EMPTY AFTER REMOVAL \n";
    *this << "SAME GEO TRAVEL SEGS CONTAINED IN FARE USAGE\n";
    break;

  case Diag306Type::ORIGIN_DAY_OF_WEEK_FAIL:
    *this << "MINSTAY SKIPPED: \n";
    *this << "ORIGIN DAY OF WEEK DOES NOT MATCH MINSTAY DOW\n";
    break;

  case Diag306Type::FM_END_IS_LAST_TVL_SEG:
    *this << "MINSTAY PASSED\n";
    break;

  case Diag306Type::SOFTPASS_MORE_THAN_ONE_TVL_SEG:
    *this << "MINSTAY SOFTPASS: "
          << "MORE THAN ONE TRAVEL SEGMENTS\n";
    break;
  case Diag306Type::SOFTPASS_RTW_REQUEST:
    *this << "MINSTAY SOFTPASS: RTW REQUEST\n";
    break;

  case Diag306Type::SOFTPASS_FM_DIRECT_NOT_OUTBOUND:
    *this << "MINSTAY SOFTPASS: "
          << "FAREMARKET DIRECTIONALITY NOT OUTBOUND - NEED REVALIDATION\n";
    break;

  case Diag306Type::SOFTPASS_PAXTYPE_FARE_ONEWAY:
    *this << "MINSTAY SOFTPASS: PAXTYPE FARE IS ONEWAY\n";
    break;

  case Diag306Type::FAIL_RETURN_DATE:
    *this << "MINSTAY FAILED: - EMPTY OR INVALID RETURN DATE\n";
    break;

  case Diag306Type::FAIL_FARE_COMP:
    *this << " MINIMUM STAY: FAILED "
          << "- GET TSI SCOPE FROM GEO RULE ITEM 995\n";
    break;

  case Diag306Type::FAIL_ITIN_TVL_SEG_EMPTY:
    *this << " MINSTAY: FAILED - ITIN TRAVEL SEG EMPTY\n";
    break;

  case Diag306Type::SOFTPASS_NEED_REVALIDATION_FOR_PU:
    *this << "\nMINSTAY SOFTPASS: NEED REVALIDATION FOR PRICING UNIT\n";
    break;

  case Diag306Type::FAIL_GEO_TLB_ITEM:
    *this << "MINSTAY FAILED: ";

    if (minStayRule->geoTblItemNoFrom() == 0)
      *this << "MINSTAY GEO TBL ITEM NO FROM IS ZERO\n";

    if (minStayRule->geoTblItemNoTo() == 0)
      *this << "MINSTAY GEO TBL ITEM NO TO IS ZERO\n";
    break;

  case Diag306Type::SOFTPASS_DOMESTIC_OR_TRANSBORDER_FM:
    *this << "MINSTAY SOFTPASS: DOMESTIC OR TRANSBORDER FARE MARKET\n";
    break;
  }
  flushMsg();
}

void
Diag306Collector::printDataUnavailableFailed(Record3ReturnTypes retVal,
                                             const MinStayRestriction* minStayRule)
{
  if (LIKELY(!_active))
    return;
  if (retVal == SKIP)
    *this << " MINSTAY: SKIPPED";
  else
    *this << " MINSTAY: FAILED";

  *this << "- UNAVAILABLE DATA TAG : " << minStayRule->unavailTag() << "\n";
  flushMsg();
}
void
Diag306Collector::formatDiagnostic(const TravelSeg* travelSeg, Record3ReturnTypes retval)
{
  if (LIKELY(!_active))
    return;

  const Loc& orig = *(travelSeg)->origin();
  const DateTime& depDate = travelSeg->departureDT();

  std::string retStr;

  if (fallback::fixed::APO29538_StopoverMinStay())
  {
    if (retval == PASS)
      retStr = "PASS";
    else if (retval == FAIL)
      retStr = "FAIL";
    else if (retval == SKIP)
      retStr = "SKIP";
  }
  else
  {
    switch (retval)
    {
    case PASS:
      retStr = "PASS";
      break;

    case SOFTPASS:
      retStr = "SOFTPASS";
      break;

    case FAIL:
      retStr = "FAIL";
      break;

    case SKIP:
      retStr = "SKIP";
      break;

    default:
      break;
    }
  }

  *this << "ITINERARY DEPARTURE DATE AND TIMES" << std::endl;
  *this << "DEPARTURE      - " << orig.loc().c_str() << " " << depDate.dateToSqlString().c_str()
        << " ";

  std::string timeStr = depDate.timeToString(HHMM, ":");
  *this << timeStr;
  *this << " " << retStr.c_str() << std::endl;
  *this << "DAY OF WEEK    - " << WEEKDAYS_UPPER_CASE[depDate.dayOfWeek()] << std::endl;
  flushMsg();
}

void
Diag306Collector::displayMinStayRules(const MinStayRestriction& minStayRule,
                                      const PeriodOfStay& minStayPeriod)
{
  if (LIKELY(!_active))
    return;

  *this << "CATEGORY 6 RULE DATA\n";
  if (minStayPeriod.isValid())
  {
    if (minStayPeriod.isDayOfWeek())
    {
      *this << "MINSTAY - " << minStayPeriod.unit() << " " << std::string(minStayPeriod);
    }
    else
    {
      *this << "MINSTAY - " << (int)minStayPeriod << " ";

      if (minStayPeriod.unit() == PeriodOfStay::DAYS)
        *this << "DAY";
      else if (minStayPeriod.unit() == PeriodOfStay::MINUTES)
        *this << "MINUTE";
      else if (minStayPeriod.unit() == PeriodOfStay::HOURS)
        *this << "HOUR";
      else if (minStayPeriod.unit() == PeriodOfStay::MONTHS)
        *this << "MONTH";
    }
  }
  else
    *this << "MINSTAY - N/A";

  if (!minStayRule.originDow().empty())
  {
    if (minStayRule.originDow().size() > 1)
    {
      *this << std::endl;
      *this << "ORIGIN DOW: ";

      for (const auto& origDow : minStayRule.originDow())
      {
        int dayOfWeek = origDow - '0';

        // SUN is zeroeth element in WEEKDAYS array but 7 in database
        //
        if (dayOfWeek == 7)
          *this << WEEKDAYS_UPPER_CASE[0];
        else
          *this << WEEKDAYS_UPPER_CASE[dayOfWeek];

        *this << " ";
      }
    }
    else
    {
      int dayOfWeek = atoi(minStayRule.originDow().c_str());

      *this << " ORIGIN DOW: ";
      if (dayOfWeek == 7)
        *this << WEEKDAYS_UPPER_CASE[0];
      else
        *this << WEEKDAYS_UPPER_CASE[dayOfWeek];
    }
  }
  else
    *this << " ORIGIN DOW: NONE";

  *this << std::endl;

  if (!minStayRule.minStayDate().isValid())
    *this << "MINSTAY DATE - N/A ";
  else
  {
    *this << "MINSTAY DATE - ";
    *this << minStayRule.minStayDate().dateToSqlString();
    *this << " ";
  }

  *this << "FROM GEO - " << minStayRule.geoTblItemNoFrom() << "  "
        << "TO GEO - " << minStayRule.geoTblItemNoTo();
  *this << std::endl;

  *this << "TABLE 994 - " << minStayRule.overrideDateTblItemNo() << "    ";

  if ((minStayRule.earlierLaterInd() == MinMaxRulesUtils::EARLIER) ||
      (minStayRule.earlierLaterInd() == MinMaxRulesUtils::LATER))
    *this << "EARLIER/LATER IND -  " << minStayRule.earlierLaterInd();
  else
    *this << "EARLIER/LATER IND - N/A";

  if (minStayRule.tod() > 0)
  {
    *this << " TIME -  " << minStayRule.tod();
  }
  else
    *this << " TIME -  N/A" << std::endl;
  *this << std::endl;

  flushMsg();
}

void
Diag306Collector::displayReturnDates(const DateTime& earliestReturnDate)
{
  if (LIKELY(!_active))
    return;
  *this << "CALCULATED MINIMUM STAY DATES" << std::endl;
  *this << "RETURN DATE  - "
        << " ";

  if (earliestReturnDate.isValid())
    *this << earliestReturnDate.dateToString(YYYYMMDD, "-");
  else
    *this << "N/A";
  *this << std::endl;

  *this << "RETURN TIME  -  ";

  if (earliestReturnDate.isValid())
    *this << earliestReturnDate.timeToString(HHMM, ":");
  else
    *this << "N/A";

  *this << std::endl;
}

void
Diag306Collector::displayDeterminedMinStayDate(const DateTime& minStayDate)
{
  if (LIKELY(!_active))
    return;
  *this << "VALIDATE AGAINST   -  ";

  *this << minStayDate.dateToString(YYYYMMDD, "-") << " ";
  *this << minStayDate.timeToString(HHMM, ":") << std::endl;
}
}
