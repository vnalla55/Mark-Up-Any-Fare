//-------------------------------------------------------------------
//
//  File:        OriginallyScheduledFlightValidator.cpp
//  Created:     July 13, 2009
//
//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "Rules/OriginallyScheduledFlightValidator.h"

#include "Common/LocUtil.h"
#include "DataModel/RexBaseRequest.h"
#include "DataModel/RexBaseTrx.h"
#include "Diagnostic/DiagCollector.h"
#include "Util/BranchPrediction.h"

namespace tse
{

bool
OriginallyScheduledFlightValidator::validate(uint32_t itemNoR3,
                                             const std::vector<TravelSeg*>& segs,
                                             Indicator origSchedFltInd,
                                             const ResPeriod& origSchedFltPeriod,
                                             const ResUnit& origSchedFltUnit)
{
  bool isPass = match(segs, origSchedFltInd, origSchedFltPeriod, origSchedFltUnit[0]);

  if (LIKELY(!_dc))
    return isPass;

  if (_softPass)
  {
    *_dc << "ORIGINALLY SCHEDULED FLIGHT: SOFTPASS\n";
    return true;
  }

  printMainMessage(origSchedFltInd, origSchedFltPeriod, origSchedFltUnit[0]);

  if (_error)
  {
    *_dc << " ERROR: RECORD3 NO. " << itemNoR3 << " INCORRECT BYTE 44\n";
    LOG4CXX_ERROR(_logger, "R3 item No. " << itemNoR3 << " incorrect byte 44");
    return false;
  }

  *_dc << " REFUND DATE/TIME          : " << _trx.currentTicketingDT().toIsoExtendedString()
       << "\n";

  if (_trx.currentTicketingDT() != _reissueDT)
    *_dc << " ADJUSTED REFUND DATE/TIME : " << _reissueDT.toIsoExtendedString() << "\n";

  if (_when == BEFORE)
    *_dc << " REFUND ALLOWED ON/BEFORE  : " << _limitDT.toIsoExtendedString() << "\n";
  else if (origSchedFltPeriod.empty())
    *_dc << " REFUND ALLOWED ON/AFTER   : " << _limitDT.toIsoExtendedString() << "\n";
  else
    *_dc << " REFUND ALLOWED BETWEEN:\n  " << _limitDT.toIsoExtendedString() << " AND "
         << _latestDT.toIsoExtendedString() << "\n";

  if (!isPass)
    *_dc << " FAILED ITEM " << itemNoR3 << " - ORIG SCHED FLT RESTR NOT MET\n";

  return isPass;
}

bool
OriginallyScheduledFlightValidator::validate(uint32_t itemNo,
                                             int seqNo,
                                             const std::vector<TravelSeg*>& segs,
                                             Indicator origSchedFltInd,
                                             const std::string& origSchedFltPeriod,
                                             Indicator origSchedFltUnit)
{
  bool isPass = match(segs, origSchedFltInd, origSchedFltPeriod, origSchedFltUnit);

  if (LIKELY(!_dc))
    return isPass;

  *_dc << " ORIGINALLY SCHEDULED FLIGHT CHECK: " << origSchedFltInd;
  if (!origSchedFltPeriod.empty())
    *_dc << " " << origSchedFltPeriod << origSchedFltUnit;
  *_dc << "\n";

  if (_error)
  {
    LOG4CXX_ERROR(_logger,
                  "T988 item No. " << itemNo << " seq No. " << seqNo << " incorrect byte 32");
    *_dc << "\nERROR: TABLE 988 SEQ NO " << seqNo << " INCORRECT BYTE 32\n";
    return false;
  }

  if (!isPass)
  {
    *_dc << "  SEQ " << seqNo << ": BYTE 32 CHECK FAILED\n"
         << "    ORIG SCHED: " << origSchedFltInd;
    if (!origSchedFltPeriod.empty())
      *_dc << " " << origSchedFltPeriod << origSchedFltUnit;
    *_dc << "\n";

    if (_when == BEFORE)
      *_dc << "    CHANGES ALLOWED ON/BEFORE " << _limitDT.toIsoExtendedString() << "\n";
    else if (origSchedFltPeriod.empty())
      *_dc << "    CHANGES ALLOWED ON/AFTER " << _limitDT.toIsoExtendedString() << "\n";
    else
      *_dc << "    CHANGES ALLOWED BETWEEN\n"
           << "      " << _limitDT.toIsoExtendedString() << " AND "
           << _latestDT.toIsoExtendedString() << "\n";
  }

  return isPass;
}

void
OriginallyScheduledFlightValidator::printMainMessage(Indicator ind,
                                                     const std::string& period,
                                                     Indicator unit) const
{
  *_dc << "ORIGINALLY SCHEDULED FLIGHT: ";
  if (int per = std::atoi(period.c_str()))
    *_dc << per << ' ' << toString(unit, per > 1) << ' ';
  *_dc << toString(ind) << "\n";
}

std::string
OriginallyScheduledFlightValidator::toString(Indicator ind) const
{
  switch (ind)
  {
  case NOT_APPLY: // Blank
    return "NOT APPLY";

  case ANYTIME_AFTER: // A
    return "ANYTIME AFTER";

  case ANYTIME_BEFORE: // B
    return "ANYTIME BEFORE";

  case DAY_BEFORE: // D
    return "DAY BEFORE";

  case DAY_AFTER: // L
    return "DAY AFTER";

  case SAME_DAY_OR_LATER: // O
    return "SAME DAY OR LATER";

  case SAME_DAY_OR_EARLIER: // X
    return "SAME DAY OR EARLIER";
  }
  return std::string() += ind;
}

std::string
OriginallyScheduledFlightValidator::toString(Indicator ind, bool plural) const
{
  std::string suffix = plural ? "S" : "";
  switch (ind)
  {
  case DAY: // D
    return "DAY" + suffix;

  case HOUR: // H
    return "HOUR" + suffix;

  case MONTH: // M
    return "MONTH" + suffix;

  case MINUTE: // N
    return "MINUTE" + suffix;
  }
  return std::string() += ind;
}

namespace
{

struct IsUnflownChanged : public std::unary_function<const TravelSeg*, bool>
{
  bool operator()(const TravelSeg* seg) const
  {
    return (seg->unflown() && seg->segmentType() == Air &&
            seg->changeStatus() == TravelSeg::CHANGED);
  }
};

typedef boost::gregorian::date_duration Days;
typedef boost::gregorian::months Months;
typedef boost::gregorian::date Date;

static const TimeDuration
h23m59(23, 59, 0);
static const Days
oneDay(1),
    twoDays(2);
static const TimeDuration oneMinute = Minutes(1);
}

bool
OriginallyScheduledFlightValidator::match(const std::vector<TravelSeg*>& segs,
                                          Indicator origSchedFltInd,
                                          const std::string& origSchedFltPeriod,
                                          Indicator origSchedFltUnit)
{
  const int period = std::atoi(origSchedFltPeriod.c_str());

  if (origSchedFltInd == NOT_APPLY)
    return true;

  const std::vector<TravelSeg*>::const_iterator firstUnflownChanged =
      std::find_if(segs.begin(), segs.end(), IsUnflownChanged());

  if ((_softPass = firstUnflownChanged == segs.end())) // fully flon fare component
    return true;

  const Loc& departureLoc = *(*firstUnflownChanged)->origin();
  const DateTime& departureDT = (*firstUnflownChanged)->departureDT();

  _reissueDT = getReissueDate(departureLoc, departureDT);

  const Date departureDate = departureDT.date();
  DateTime referenceDT;

  _error = false;

  switch (origSchedFltInd)
  {
  case ANYTIME_BEFORE: // B
    referenceDT = departureDT;
    _limitDT = departureDT - oneMinute;
    _when = BEFORE;
    break;

  case DAY_BEFORE: // D
    referenceDT = DateTime(departureDate - oneDay);
    _limitDT = DateTime(departureDate) - oneMinute;
    _when = BEFORE;
    break;

  case SAME_DAY_OR_EARLIER: // X
    referenceDT = DateTime(departureDate);
    _limitDT = DateTime(departureDate, h23m59);
    _when = BEFORE;
    break;

  case ANYTIME_AFTER: // A
    referenceDT = departureDT;
    _limitDT = (origSchedFltUnit == MINUTE && period == 0
                    ? // Probably no carrier will code such a combination
                    DateTime(departureDT)
                    : departureDT + oneMinute); // but it is mentioned in ATPCO Cat31 Data App, so
                                                // we support it.
    _when = AFTER;
    break;

  case SAME_DAY_OR_LATER: // O
    referenceDT = DateTime(departureDate + oneDay);
    if (origSchedFltUnit == DAY || origSchedFltUnit == MONTH)
      referenceDT -= oneMinute;
    _limitDT = DateTime(departureDate);
    _when = AFTER;
    break;

  case DAY_AFTER: // L
    referenceDT = DateTime(departureDate + twoDays);
    if (origSchedFltUnit == DAY || origSchedFltUnit == MONTH)
      referenceDT -= oneMinute;
    _limitDT = DateTime(departureDate + oneDay);
    _when = AFTER;
    break;

  default:
    return !(_error = true);
  }

  bool isPass = false;

  if (_when == BEFORE)
  {
    if (!origSchedFltPeriod.empty())
      _limitDT =
          countBackwards(referenceDT, period, origSchedFltUnit, origSchedFltInd == ANYTIME_BEFORE);
    isPass = _reissueDT <= _limitDT;
  }
  else // when == AFTER
  {
    _latestDT = countForward(referenceDT, period, origSchedFltUnit);
    isPass = _reissueDT >= _limitDT && (origSchedFltPeriod.empty() || _reissueDT <= _latestDT);
  }

  return isPass;
}

DateTime
OriginallyScheduledFlightValidator::countForward(const DateTime& refDT, int period, Indicator unit)
    const
{
  if (period == 0)
  {
    switch (unit)
    {
    case MINUTE:
      return refDT;

    case HOUR:
      return DateTime(refDT.date(), TimeDuration(refDT.hours(), 59, 0));

    case DAY:
      return DateTime(refDT.date(), h23m59);

    case MONTH:
      return DateTime(refDT.date().end_of_month(), h23m59);
    }
  }
  else
  {
    switch (unit)
    {
    case MINUTE:
      return refDT + Minutes(period);

    case HOUR:
      return refDT + Hours(period);

    case DAY:
      return DateTime(refDT.date() + Days(period), h23m59);

    case MONTH:
      Date tmpDate = Date(refDT.year(), refDT.month(), 1) + Months(period);
      tmpDate = DateTime::endOfMonthDay(tmpDate.year(), tmpDate.month()) < refDT.day()
                    ? tmpDate.end_of_month()
                    : Date(tmpDate.year(), tmpDate.month(), refDT.day());
      return DateTime(tmpDate, h23m59);
    }
  }
  return refDT; // Will never get here, but avoid compiler warning.
}

DateTime
OriginallyScheduledFlightValidator::countBackwards(const DateTime& refDT,
                                                   int period,
                                                   Indicator unit,
                                                   bool doNotUseDayBoundary) const
{
  if (doNotUseDayBoundary && period == 0)
    return refDT;

  switch (unit)
  {
  case MINUTE:
    return refDT - Minutes(period);

  case HOUR:
    return refDT - Hours(period);

  case DAY:
    return DateTime(refDT.date() - Days(period), h23m59);

  case MONTH:
    Date tmpDate = Date(refDT.year(), refDT.month(), 1) - Months(period);
    tmpDate = DateTime::endOfMonthDay(tmpDate.year(), tmpDate.month()) < refDT.day()
                  ? tmpDate.end_of_month()
                  : Date(tmpDate.year(), tmpDate.month(), refDT.day());
    return DateTime(tmpDate, h23m59);
  }
  return refDT; // Will never get here, but avoid compiler warning.
}

DateTime
OriginallyScheduledFlightValidator::getReissueDate(const Loc& departureLoc,
                                                   const DateTime& departureDT) const
{
  short utcoffset = 0;
  const Loc& saleLoc = *_trx.currentSaleLoc();

  if (LocUtil::getUtcOffsetDifference(saleLoc,
                                      departureLoc,
                                      utcoffset,
                                      _trx.dataHandle(),
                                      departureDT,
                                      _trx.currentTicketingDT()))
    return _trx.currentTicketingDT() - Minutes(utcoffset);

  LOG4CXX_ERROR(_logger,
                "getUtcOffsetDifference between " << saleLoc.loc() << " and " << departureLoc.loc()
                                                  << " failed !!!");
  return _trx.currentTicketingDT();
}
}
