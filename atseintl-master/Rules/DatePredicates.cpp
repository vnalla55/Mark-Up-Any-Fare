#include "Rules/DatePredicates.h"

#include "DataModel/AirSeg.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "Rules/Debug.h"
#include "Rules/RuleConst.h"

using namespace std;

namespace tse
{
IsDate::IsDate()
  : Predicate("IsDate"),
    _year1(0),
    _month1(0),
    _day1(0),
    _year2(0),
    _month2(0),
    _day2(0),
    _itemCat(0)
{
}

void
IsDate::initialize(int year1, int month1, int day1, int year2, int month2, int day2)
{
  if (year1 == ANY_YEAR)
    _year1 = ANY_YEAR;
  else if (year1 <= 50)
    _year1 = year1 + 2000;
  else
    _year1 = year1 + 1900;

  _month1 = month1;
  _day1 = day1;

  if (year2 == ANY_YEAR)
    _year2 = ANY_YEAR;
  else if (year2 <= 50)
    _year2 = year2 + 2000;
  else
    _year2 = year2 + 1900;

  _month2 = month2;
  _day2 = day2;
}

void
IsDate::initialize(
    int year1, int month1, int day1, int year2, int month2, int day2, uint32_t itemCat)
{
  if (year1 == ANY_YEAR)
    _year1 = ANY_YEAR;
  else if (LIKELY(year1 <= 50))
    _year1 = year1 + 2000;
  else
    _year1 = year1 + 1900;

  _month1 = month1;
  _day1 = day1;

  if (year2 == ANY_YEAR)
    _year2 = ANY_YEAR;
  else if (LIKELY(year2 <= 50))
    _year2 = year2 + 2000;
  else
    _year2 = year2 + 1900;

  _month2 = month2;
  _day2 = day2;
  _itemCat = itemCat;
}

string
IsDate::toString(int level) const
{
  string s = printTabs(level);
  stringstream temp;
  temp << _year1 << "-" << _month1 << "-" << _day1 << ", " << _year2 << "-" << _month2 << "-"
       << _day2;

  s += _name + " (" + temp.str() + ")\n";

  return s;
}

PredicateReturnType
IsDate::test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx)
{
  PredicateReturnType retval;
  PredicateOutputPrinter p(retval.valid, "IsDate");
  retval.valid = FAIL;
  retval->matchedSegments.assign(itinerary.size(), false);

  NoPNRPricingTrx* noPnrTrx =
      const_cast<NoPNRPricingTrx*>(dynamic_cast<const NoPNRPricingTrx*>(&trx));

  vector<TravelSeg*>::const_iterator it;
  it = itinerary.begin();
  FlightNumber fltNo = 0;
  CarrierCode cxr;
  for (unsigned i = 0; it != itinerary.end(); ++it, ++i)
  {
    DateTime departureDT = (*it)->departureDT();

    // for NoPNR Cat11 processing - segment date may have to be updated
    if (UNLIKELY(noPnrTrx && _itemCat == RuleConst::BLACKOUTS_RULE))
    {
      if (trx.itin().front()->dateType() == Itin::NoDate)
      {
        departureDT = DateTime::localTime();
      }
      else
      {
        // update segment dates if necessary
        noPnrTrx->updateOpenDateIfNeccesary((*it), departureDT);
      }
    }

    int year1, year2;
    if (_year1 == ANY_YEAR)
      year1 = departureDT.date().year();
    else
      year1 = _year1;

    if (_year2 == ANY_YEAR)
      year2 = departureDT.date().year();
    else
      year2 = _year2;

    DateTime date1(year1, _month1, _day1);
    DateTime date2(year2, _month2, _day2);

    AirSeg* airSeg = dynamic_cast<AirSeg*>(*it);
    // lint -e{530}
    if (i > 0 && airSeg && airSeg->flightNumber() == fltNo && airSeg->carrier() == cxr)
      continue; // It's just intermediate stop

    // check OPEN segment.
    // The rule should pass if one or more open segments without a date
    //    after a segment with a date (at the end of an itinerary) and
    //    there are no more a dated TravelSegs in Itin.
    //  otherwise, validate as ussualy with date+1(done in ItinAnalizer).

    if (UNLIKELY((*it)->segmentType() == Open && (*it)->openSegAfterDatedSeg()))
    { // Pass open segment.
      // retval.valid = PASS;
      // retval->matchedSegments[i] = true;
      continue;
    }
    else if (departureDT.date() == date1.date() || departureDT.date() == date2.date())
    {
      retval.valid = PASS;
      retval->matchedSegments[i] = true;
    }
    if (LIKELY(airSeg))
    {
      fltNo = airSeg->flightNumber();
      cxr = airSeg->carrier();
    }
  }

  return retval;
}

/////////////////////

IsDateBetween::IsDateBetween()
  : Predicate("IsDateBetween"),
    _fromYear(0),
    _fromMonth(0),
    _fromDay(0),
    _toYear(0),
    _toMonth(0),
    _toDay(0),
    _itemCat(0)
{
}

void
IsDateBetween::initialize(
    int fromYear, int fromMonth, int fromDay, int toYear, int toMonth, int toDay)
{
  if (fromYear != ANY_YEAR)
  {
    /* @TODO In what format come data from DBAccess?? Is
    *  it correct to assume that <50 -> 20xx and >50 -> 19xx ??
    */
    if (fromYear <= 50)
      _fromYear = fromYear + 2000; // This will be problem after 2050
    else
      _fromYear = fromYear + 1900;
  }
  else
  {
    _fromYear = ANY_YEAR;
  }

  _fromMonth = fromMonth;
  _fromDay = fromDay;

  if (toYear != ANY_YEAR)
  {
    /* @TODO In what format come data from DBAccess?? Is
    *  it correct to assume that <50 -> 20xx and >50 -> 19xx ??
    */
    if (toYear <= 50)
      _toYear = toYear + 2000;
    else
      _toYear = toYear + 1900;
  }
  else
  {
    _toYear = ANY_YEAR;
  }

  _toMonth = toMonth;
  _toDay = toDay;
}

void
IsDateBetween::initialize(
    int fromYear, int fromMonth, int fromDay, int toYear, int toMonth, int toDay, uint32_t itemCat)
{
  if (fromYear != ANY_YEAR)
  {
    /* @TODO In what format come data from DBAccess?? Is
    *  it correct to assume that <50 -> 20xx and >50 -> 19xx ??
    */
    if (LIKELY(fromYear <= 50))
      _fromYear = fromYear + 2000; // This will be problem after 2050
    else
      _fromYear = fromYear + 1900;
  }
  else
  {
    _fromYear = ANY_YEAR;
  }

  _fromMonth = fromMonth;
  _fromDay = fromDay;

  if (toYear != ANY_YEAR)
  {
    /* @TODO In what format come data from DBAccess?? Is
    *  it correct to assume that <50 -> 20xx and >50 -> 19xx ??
    */
    if (toYear <= 50)
      _toYear = toYear + 2000;
    else
      _toYear = toYear + 1900;
  }
  else
  {
    _toYear = ANY_YEAR;
  }

  _toMonth = toMonth;
  _toDay = toDay;
  _itemCat = itemCat;
}

string
IsDateBetween::toString(int level) const
{
  string s = printTabs(level);
  stringstream temp;
  temp << _fromYear << "-" << _fromMonth << "-" << _fromDay << " -> " << _toYear << "-" << _toMonth
       << "-" << _toDay;

  s += _name + " (" + temp.str() + ")\n";

  return s;
}

PredicateReturnType
IsDateBetween::test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx)
{
  PredicateReturnType retval;
  PredicateOutputPrinter p(retval.valid, "IsDateBetween");
  retval.valid = FAIL;
  retval->matchedSegments.assign(itinerary.size(), false);

  NoPNRPricingTrx* noPnrTrx =
      const_cast<NoPNRPricingTrx*>(dynamic_cast<const NoPNRPricingTrx*>(&trx));

  vector<TravelSeg*>::const_iterator it;
  it = itinerary.begin();
  for (unsigned i = 0; it != itinerary.end(); ++it, ++i)
  {
    if (_fromYear == ANY_YEAR)
    {
      _fromYear = 0;
    }

    if (_toYear == ANY_YEAR)
    {
      _toYear = 0;
    }

    // check OPEN segment.
    // The rule should pass if one or more open segments without a date
    //    after a segment with a date (at the end of an itinerary) and
    //    there are no more a dated TravelSegs in Itin.
    //  otherwise, validate as ussualy with date+1(done in ItinAnalizer).
    //  doesn't apply to NoPnr (WQ) transactions
    if (UNLIKELY(!noPnrTrx && (*it)->segmentType() == Open && (*it)->openSegAfterDatedSeg()))
    { // Pass open segment.
      // retval.valid = PASS;
      // retval->matchedSegments[i] = true;
      continue;
    }

    else if (LIKELY(_itemCat == RuleConst::BLACKOUTS_RULE))
    {
      bool isInRange = false;
      if (LIKELY(!noPnrTrx)) // it is not a NoPNR transaction
      {
        if (LIKELY((*it)->earliestDepartureDT().isEmptyDate()))
          isInRange = (*it)->departureDT().isBetween(
              _fromYear, _fromMonth, _fromDay, _toYear, _toMonth, _toDay);
        else
          isInRange = isWholeRangeBetween(*it);
      }
      else // for NoPNR transaction, compare (possibly updated) departure date
      {
        DateTime checkDate;

        if (trx.itin().front()->dateType() == Itin::NoDate)
        {
          // for itineraries w/o date, use current
          checkDate = DateTime::localTime();
        }
        else
        {
          checkDate = (*it)->departureDT();
          noPnrTrx->updateOpenDateIfNeccesary((*it), checkDate);
        }

        isInRange =
            (checkDate.isBetween(_fromYear, _fromMonth, _fromDay, _toYear, _toMonth, _toDay));
      }

      if (isInRange)
      {
        retval.valid = PASS;
        retval->matchedSegments[i] = true;
      }
    }
    else if ((*it)->departureDT().isBetween(
                 _fromYear, _fromMonth, _fromDay, _toYear, _toMonth, _toDay))
    {
      retval.valid = PASS;
      retval->matchedSegments[i] = true;
    }
  }

  return retval;
}

bool
IsDateBetween::isWholeRangeBetween(const TravelSeg* seg) const
{
  return seg->earliestDepartureDT().isRangeInBetween(
      _fromYear, _fromMonth, _fromDay, _toYear, _toMonth, _toDay, seg->latestDepartureDT());
}

IsDateRangeBetween::IsDateRangeBetween() : IsDateBetween() {}

bool
IsDateRangeBetween::isWholeRangeBetween(const TravelSeg* seg) const
{
  return seg->earliestDepartureDT().isBetween(
             _fromYear, _fromMonth, _fromDay, _toYear, _toMonth, _toDay) &&
         seg->latestDepartureDT().isBetween(
             _fromYear, _fromMonth, _fromDay, _toYear, _toMonth, _toDay);
}
}
