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
#pragma once
#include "Common/TseEnums.h"
#include "Diagnostic/DiagCollector.h"

namespace tse
{
class MinStayRestriction;
class TravelSeg;
class DateTime;
class PeriodOfStay;

enum class Diag306Type : uint8_t
{ NO_MINIMUM_STAY,
  MIN_STAY_FAILED,
  PASS_ONEWAY,
  SKIP_GEO_RULE,
  PASS_OPEN_SEGMENT,
  SAME_GEO_IN_FARE_USAGE,
  ORIGIN_DAY_OF_WEEK_FAIL,
  FM_END_IS_LAST_TVL_SEG,
  SOFTPASS_MORE_THAN_ONE_TVL_SEG,
  SOFTPASS_RTW_REQUEST,
  SOFTPASS_FM_DIRECT_NOT_OUTBOUND,
  SOFTPASS_PAXTYPE_FARE_ONEWAY,
  FAIL_RETURN_DATE,
  FAIL_FARE_COMP,
  FAIL_ITIN_TVL_SEG_EMPTY,
  SOFTPASS_NEED_REVALIDATION_FOR_PU,
  FAIL_GEO_TLB_ITEM,
  SOFTPASS_DOMESTIC_OR_TRANSBORDER_FM };

class Diag306Collector : public DiagCollector
{
public:
  explicit Diag306Collector(Diagnostic& root) : DiagCollector(root) {}
  Diag306Collector() = default;
  void printDiagData(Diag306Type diag306Type, const MinStayRestriction* minStayRule = nullptr);
  void printDataUnavailableFailed(Record3ReturnTypes retVal, const MinStayRestriction* minStayRule);
  void formatDiagnostic(const TravelSeg* travelSeg, Record3ReturnTypes retval);
  void
  displayMinStayRules(const MinStayRestriction& minStayRule, const PeriodOfStay& minStayPeriod);
  void displayReturnDates(const DateTime& earliestReturnDate);
  void displayDeterminedMinStayDate(const DateTime& minStayDate);
};
} // tse namespace
