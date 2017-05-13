#include "Rules/FlightAppPredicates.h"

#include "Common/FallbackUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DBAccess/AirlineAllianceCarrierInfo.h"
#include "DBAccess/Loc.h"
#include "Rules/FlightApplication.h"

#include <boost/date_time/posix_time/posix_time.hpp>

using namespace std;
using namespace boost;

namespace tse
{
static std::string A340_ALL_SERIES = "340";

std::string
AndFlight::toString(int level) const
{
  std::string s = Predicate::toString(level);
  if (_cond1)
    s += _cond1->toString(level + 1);
  if (_cond2)
    s += _cond2->toString(level + 1);

  return s;
}

PredicateReturnType
AndFlight::test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx)
{
  PredicateReturnType retval;
  PredicateReturnType cond1;
  PredicateReturnType cond2;
  PredicateOutputPrinter p(retval.valid, "AndFlight");
  retval->matchedSegments.assign(itinerary.size(), false);

  if (LIKELY(_cond1))
    cond1 = _cond1->test(itinerary, trx);
  if (LIKELY(_cond2))
    cond2 = _cond2->test(itinerary, trx);

  // No flight matches any of the conditions
  if (cond1.valid == FAIL || cond2.valid == FAIL)
    return retval;

  // Only one flight - matches both conditions
  if (itinerary.size() == 1)
  {
    PredicateReturnType::iterator first = cond1->matchedSegments.begin();
    PredicateReturnType::iterator second = cond2->matchedSegments.begin();
    bool theSame = true;
    while (first != cond1->matchedSegments.end())
    {
      theSame &= !(*first ^ *second);
      ++first;
      ++second;
    }
    if (LIKELY(theSame))
    {
      retval.valid = FAIL;
      return retval;
    }
  }

  // If above not true, condition is met.
  //
  // SOFTPASS if any of the conditions is SOFTPASS
  if (UNLIKELY((cond1.valid == SOFTPASS) || (cond2.valid == SOFTPASS)))
  {
    retval.valid = SOFTPASS;
  }
  else
  {
    retval.valid = PASS;
  }

  // Combine response
  //
  /// @todo Why vector.insert dumps core??

  PredicateReturnType::iterator it1, it2, retit;
  it1 = cond1->matchedSegments.begin();
  it2 = cond2->matchedSegments.begin();
  retit = retval->matchedSegments.begin();
  size_t numMatchedSegs = 0;
  for (; it1 != cond1->matchedSegments.end(); ++it1, ++it2, ++retit)
  {
    if ((*retit = *it1 | *it2))
      numMatchedSegs++;
  }

  if (numMatchedSegs < 2)
  {
    retval.valid = FAIL; // need at least two segments
  }
  return retval;
}

/////////////////////////////
/*
std::string
ConnectingFlights::toString(int level) const
{
    std::string s = Predicate::toString(level);
    if (_cond1)
        s += _cond1->toString(level + 1);
    if (_cond2)
        s += _cond2->toString(level + 1);

    return s;
}
*/

// PredicateReturnType
// ConnectingFlights::test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx)
// {
//     PredicateReturnType retval;
//     PredicateReturnType cond1;
//     PredicateReturnType cond2;
//     PredicateOutputPrinter p(retval.valid, "ConnectingFlight");
//     retval->matchedSegments.assign(itinerary.size(), false);

//     if (_cond1) cond1 = _cond1->test(itinerary, trx);
//     if (_cond2) cond2 = _cond2->test(itinerary, trx);

//     // No flight matches any of the conditions
//     if ( cond1.valid == FAIL || cond2.valid == FAIL ) return retval;

//     // ATPco sais: "Must have all flights from Flight1 immediately prior
//     //  all flights from Flight2"
//     /** @todo What if:
//      *  @verbatim
//         flight1 -> flight2 -> flight3
//          UA100      AA100      UA101
//         Rec3:
//          UA ****  1  AA ****
//         @endverbatim
//      * Fare will be failed. Should it be passed??
//      */

//     retval.valid = PASS;
//     PredicateReturnType::iterator first = cond2->matchedSegments.begin();
//     //bool guard = !*first;
//     // Find first true and than first false. Guard rules conditions change
//     while (first != cond2->matchedSegments.end() && !*first)
//            //&& (guard || *first))
//     {
//         //if (*first) guard = false;
//         ++first;
//     }
//     //--first;
//     PredicateReturnType::reverse_iterator last = cond1->matchedSegments.rbegin();
//     //guard = !*last;//true;
//     // Find first true and than first false. Guard rules conditions change
//     while(last != cond1->matchedSegments.rend()
//           && !*last)
//           //&& (guard || *last))
//     {
//         //if (*last) guard = false;
//         ++last;
//     }
//     //--last;
// //     cout << "\tseg1: " <<distance(cond1->matchedSegments.begin(), last.base() -1)
// //          << " seg2: " << distance(cond2->matchedSegments.begin(), first)
// //          << endl;
//     if (distance(cond2->matchedSegments.begin(), first) - 1 !=
//         distance(cond1->matchedSegments.begin(), last.base() - 1))
//         retval.valid = FAIL;

//     /// @todo Not needed to join matched segments here?
//     return retval;
// }

/////////////////////////////

// std::string
// NotConnectingFlights::toString(int level) const
// {
//     std::string s = Predicate::toString(level);
//     if (_cond1)
//         s += _cond1->toString(level + 1);
//     if (_cond2)
//         s += _cond2->toString(level + 1);

//     return s;
// }

// PredicateReturnType
// NotConnectingFlights::test(const std::vector<TravelSeg*>& itinerary)
// {
//     PredicateReturnType retval;
//     PredicateReturnType cond1;
//     PredicateReturnType cond2;
//     PredicateOutputPrinter p(retval.valid, "NotConnectingFlight");
//     retval->matchedSegments.assign(itinerary.size(), false);

//     if (_cond1) cond1 = _cond1->test(itinerary, trx);
//     if (_cond2) cond2 = _cond2->test(itinerary, trx);

//     // No flight matches any of the conditions
//     if (!cond1.valid || !cond2.valid) {
//         retval.valid = true;
//         return retval;
//     }

//     retval.valid = true;
//     PredicateReturnType::iterator first, second;
//     first = cond1->matchedSegments.begin();
//     second = cond2->matchedSegments.begin();
//     bool lastSegment1 = false;
//     bool lastSegment2 = false;
//     for (; first != cond1->matchedSegments.end(); ++first, ++second) {
//         if (lastSegment1 && !*first && *second)
//             retval.valid = false;
//         if (lastSegment2 && !*second && *first)
//             retval.valid = true;
//         lastSegment1 = *first;
//         lastSegment2 = *second;
//     }

//     /// @todo Not needed to join matched segments here?
//     return retval;
// }

/////////////////////////////

void
AndConnectingFlights::initialize(Predicate* and1,
                                 Predicate* and2,
                                 Predicate* conn,
                                 bool allConnecting,
                                 bool andFirst,
                                 bool negAppl)
{
  _and1 = and1;
  _and2 = and2;
  _conn = conn;
  _allConnecting = allConnecting;
  _andFirst = andFirst;
  _negAppl = negAppl;
}

string
AndConnectingFlights::toString(int level) const
{
  string s;
  int inc = 1;
  if (_negAppl)
  {
    s = printTabs(level);
    s += "Not\n";
    inc = 2;
  }
  s += Predicate::toString(level + inc - 1);
  if (_andFirst)
  {
    if (_and1)
      s += _and1->toString(level + inc);
    if (_and2)
      s += _and2->toString(level + inc);
    s += printTabs(level + inc) + "-> to\n";
    if (_conn)
      s += _conn->toString(level + inc);
  }
  else
  {
    if (_conn)
      s += _conn->toString(level + inc);
    s += printTabs(level + inc) + "-> to\n";
    if (_and1)
      s += _and1->toString(level + inc);
    if (_and2)
      s += _and2->toString(level + inc);
  }

  return s;
}

PredicateReturnType
AndConnectingFlights::test(const vector<TravelSeg*>& itinerary, const PricingTrx& trx)
{
  PredicateReturnType retval;
  PredicateReturnType conn;
  PredicateReturnType and1;
  PredicateReturnType and2;
  PredicateOutputPrinter p(retval.valid, "AndConnectingFlight");
  retval->matchedSegments.assign(itinerary.size(), false);

  const uint16_t numSegsNeeded = (_and2) ? 3 : 2;
  if (itinerary.size() < numSegsNeeded)
  {
    retval.valid = _negAppl ? PASS : FAIL;
    if (retval.valid == PASS)
      retval->matchedSegments.assign(itinerary.size(), true);
    return retval;
  }

  if (_conn)
    conn = _conn->test(itinerary, trx);
  if (_and1)
    and1 = _and1->test(itinerary, trx);
  if (_and2)
    and2 = _and2->test(itinerary, trx);

  // No flight matches any of the conditions
  if (conn.valid == FAIL || and1.valid == FAIL || (_and2 && and2.valid == FAIL))
  {
    retval.valid = _negAppl ? PASS : FAIL;
    if (retval.valid == PASS)
      retval->matchedSegments.assign(itinerary.size(), true);
    return retval;
  }

  // Any component is SOFTPASS (because of OpenSeg), we do SOFTPASS
  // all matchedSegments set as true, no guarantee accurate
  if ((conn.valid == SOFTPASS) || (and1.valid == SOFTPASS) || (and2.valid == SOFTPASS))
  {
    retval.valid = SOFTPASS;
    retval->matchedSegments.assign(itinerary.size(), true);
    return retval;
  }

  // ATPCO specify that when the relation is CONNECT, we should not see
  // flight matching connected flight/flight tables after flight matching
  // connecting flight/flight tables
  // Example1, if the first three flights match
  // 1. fltTbl1 CONNECT fltTbl2 AND flt3, not flight after third flight should
  // match fltTbl1, unless it matches fltTbl2 or flt3(Cxr specified, any flt);
  //
  // 2. fltTbl1 CONNECT fltTbl2 CONNECT flt3, not flight after third flight
  // should match fltTbl1 or fltTbl2, unless it matches flt3 (Cxr, flt####)
  //
  // Example2, if the flights#2-4 match
  // fltTbl1 CONNECT fltTbl2 AND flt3, flight#1 should not match
  // fltTbl2 or flt3, unless it matches fltTbl1
  if (_and2)
  {
    PredicateReturnType::iterator and1Itr = and1->matchedSegments.begin();
    PredicateReturnType::iterator and2Itr = and2->matchedSegments.begin();
    PredicateReturnType::iterator connItr = conn->matchedSegments.begin();

    int maxOffset = itinerary.size() - numSegsNeeded;

    for (int offset = 0; offset <= maxOffset; offset++, and1Itr++, and2Itr++, connItr++)
    {
      if (_andFirst)
      {
        if (!((*and1Itr && *(and2Itr + 1)) || (*(and1Itr + 1) && *and2Itr)))
        {
          if (!_negAppl && (*connItr || *(connItr + 1))) // against order
            break;
          else
            continue;
        }

        if (!*(connItr + 2))
          continue;

        // check the rest when not _negAppl, rest should not match and1
        // or and2, unless the rest match conn
        int nextOffset = _negAppl ? (maxOffset + 1) : (offset + 1);
        for (; nextOffset <= maxOffset; nextOffset++, connItr++, and1Itr++, and2Itr++)
        {
          if (!*(connItr + 3) && (*(and1Itr + 3) || *(and2Itr + 3)))
            break;
        }

        if (nextOffset > maxOffset)
        {
          retval.valid = (_negAppl) ? FAIL : PASS;
          if (retval.valid == PASS)
            retval->matchedSegments.assign(itinerary.size(), true);
          return retval;
        }
        else
        {
          and1Itr = and1->matchedSegments.begin() + offset;
          and2Itr = and2->matchedSegments.begin() + offset;
          connItr = conn->matchedSegments.begin() + offset;

          continue;
        }
      }
      else
      { // conn first, and later and1, and2
        if (!*connItr)
        {
          if (!_negAppl && (*and1Itr || *and2Itr)) // against order
            break;
          else
            continue;
        }

        if (!((*(and1Itr + 1) && *(and2Itr + 2)) ||
              (!_allConnecting && *(and1Itr + 2) && *(and2Itr + 1))))
          continue;

        // check the rest when not _negAppl, rest should not match conn,
        // unless the rest match and2 or and1 (if relation2 was AND,
        // not CONNECTING as well)
        int nextOffset = _negAppl ? (maxOffset + 1) : (offset + 1);
        for (; nextOffset <= maxOffset; nextOffset++, connItr++, and1Itr++, and2Itr++)
        {
          if (*(and2Itr + 3))
            continue;

          if (*(and1Itr + 3))
          {
            if (_allConnecting)
              break; // against order
            else
              continue;
          }

          if (*(connItr + 3))
            break; // against order
        }

        if (nextOffset > maxOffset)
        {
          retval.valid = (_negAppl) ? FAIL : PASS;
          if (retval.valid == PASS)
            retval->matchedSegments.assign(itinerary.size(), true);
          return retval;
        }
        else
        {
          and1Itr = and1->matchedSegments.begin() + offset;
          and2Itr = and2->matchedSegments.begin() + offset;
          connItr = conn->matchedSegments.begin() + offset;

          continue;
        }
      }
    }
    retval.valid = (_negAppl) ? PASS : FAIL;
    if (retval.valid == PASS)
      retval->matchedSegments.assign(itinerary.size(), true);
    return retval;
  }
  else
  {
    // two connecting flights
    const PredicateReturnType::iterator firstFltItrBegin =
        (_andFirst) ? and1->matchedSegments.begin() : conn->matchedSegments.begin();
    const PredicateReturnType::iterator connectFltItrBegin =
        (_andFirst) ? conn->matchedSegments.begin() : and1->matchedSegments.begin();

    PredicateReturnType::iterator and1Itr = firstFltItrBegin;
    PredicateReturnType::iterator connItr = connectFltItrBegin;

    int maxOffset = itinerary.size() - numSegsNeeded;

    for (int offset = 0; offset <= maxOffset; offset++, and1Itr++, connItr++)
    {
      if (!*and1Itr)
      {
        if (!_negAppl && *connItr)
          break; // fail
        else
          continue;
      }

      if (!*(connItr + 1))
        continue;

      // check the rest when not _negAppl, rest should not match and1,
      // unless it matches conn
      //
      int nextOffset = _negAppl ? (maxOffset + 1) : (offset + 1);
      for (; nextOffset <= maxOffset; nextOffset++, connItr++, and1Itr++)
      {
        if (!*(connItr + 2) && *(and1Itr + 2))
          break;
      }

      if (nextOffset > maxOffset)
      {
        retval.valid = (_negAppl) ? FAIL : PASS;
        if (retval.valid == PASS)
          retval->matchedSegments.assign(itinerary.size(), true);
        return retval;
      }
      else
      {
        and1Itr = firstFltItrBegin + offset;
        connItr = connectFltItrBegin + offset;

        continue;
      }
    }
    retval.valid = (_negAppl) ? PASS : FAIL;
    if (retval.valid == PASS)
      retval->matchedSegments.assign(itinerary.size(), true);
    return retval;
  }
}

std::string
AllFlightsMatch::toString(int level) const
{
  std::string s = Predicate::toString(level);
  if (_pred)
    s += _pred->toString(level + 1);

  return s;
}

PredicateReturnType
AllFlightsMatch::test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx)
{
  PredicateReturnType retval;
  PredicateOutputPrinter p(retval.valid, "AllFlightsMatch");

  bool isThereSoftPass = false;
  if (LIKELY(_pred))
  {
    retval = _pred->test(itinerary, trx);
    isThereSoftPass = (retval.valid == SOFTPASS);
  }

  PredicateReturnType::iterator it = retval->matchedSegments.begin();
  int i = 0;
  for (; (it != retval->matchedSegments.end() && retval.valid != FAIL); ++it, ++i)
  {
    const TravelSegType segType = itinerary[i]->segmentType();
    if (LIKELY((segType != Arunk) // Atunk doesn't have to match
        &&
        (segType != Surface))) // Surface doesn't have to match
      retval.valid = *it ? PASS : FAIL;
  }

  if ((retval.valid == PASS) && isThereSoftPass)
  {
    retval.valid = SOFTPASS;
  }
  return retval;
}

///////////////////////

void
DayOfWeekPredicate::initialize(const std::set<DayName>& days)
{
  std::set<DayName>::const_iterator day;

  for (day = days.begin(); day != days.end(); ++day)
  {
    _days.set(*day);
  }
}

std::string
DayOfWeekPredicate::toString(int level) const
{
  std::string s;
  s += printTabs(level) + _name + " (";
  s += "Flight valid on:";
  if (_days[Mon])
    s += " Mon";
  if (_days[Tue])
    s += " Tue";
  if (_days[Wed])
    s += " Wed";
  if (_days[Thu])
    s += " Thu";
  if (_days[Fri])
    s += " Fri";
  if (_days[Sat])
    s += " Sat";
  if (_days[Sun])
    s += " Sun";
  s += ")\n";

  return s;
}

PredicateReturnType
DayOfWeekPredicate::test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx)
{
  PredicateReturnType retval;
  PredicateOutputPrinter p(retval.valid, "DayOfWeek");
  unsigned short day;
  retval->matchedSegments.assign(itinerary.size(), false);

  for (size_t segmentNo = 0; segmentNo < itinerary.size(); ++segmentNo)
  {
    posix_time::ptime tempTime
        //(posix_time::from_time_t(itinerary[segmentNo]->departureDT()));
        (itinerary[segmentNo]->departureDT());
    // posix_time::ptime start(gregorian::date(1970,1,1));
    // tempTime = start
    //    + posix_time::time_duration(0,0,itinerary[segmentNo]->departureDT(),0);
    day = tempTime.date().day_of_week().as_number();
    // day = itinerary[segmentNo]->departure.date().day_of_week().as_number();
    //         cout << "\tFlight on: " << day << " - atpco: " << boostToAtpco(day)
    //              << " (date: " << tempTime
    //              << ", date as time_t: " << itinerary[segmentNo]->departureDT()
    //              << ")" << endl;
    if (_days[boostToAtpco(day)])
    {
      retval.valid = PASS;
      retval->matchedSegments[segmentNo] = true;
    }
  }

  return retval;
}

DayOfWeekPredicate::DayName
DayOfWeekPredicate::boostToAtpco(unsigned short day) const
{
  if (day == 0)
    return Sun;
  return static_cast<DayName>(day);
}

///////////////////////////////

std::string
Via::toString(int level) const
{
  std::string s = printTabs(level);
  s += "Travel via " + _city + "\n";
  return s;
}

PredicateReturnType
Via::test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx)
{
  PredicateReturnType retval;
  retval->matchedSegments.assign(itinerary.size(), false);

  size_t segmentNo;
  for (segmentNo = 1; segmentNo < itinerary.size() - 1; ++segmentNo)
  {
    // AirSeg* segment = dynamic_cast<AirSeg*>(itinerary[segmentNo].get());
    // if (!segment) continue;
    if (itinerary[segmentNo]->origin()->loc() == _city)
    {
      retval.valid = PASS;
      retval->matchedSegments[segmentNo] = true;
    }
  }

  return retval;
}

///////////////////////////////

std::string
BetweenCities::toString(int level) const
{
  std::string s = printTabs(level);
  s += "Travel between " + _city1 + " and " + _city2 + "\n";
  return s;
}

PredicateReturnType
BetweenCities::test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx)
{
  // Find first segment in which orig == city1,
  // search from that place until dest == city2
  // then fill retval with segments from orig to dest.
  PredicateReturnType retval;
  size_t begSeg, segmentNo = 0;
  retval->matchedSegments.assign(itinerary.size(), false);
  // AirSeg* segment = dynamic_cast<AirSeg*>(itinerary[segmentNo].get());
  // if (!segment) return retval;

  while (segmentNo < itinerary.size() && itinerary[segmentNo]->origin()->loc() != _city1)
    ++segmentNo;
  begSeg = segmentNo;
  // segment = dynamic_cast<AirSeg*>(itinerary[segmentNo].get());
  // if (!segment) return retval;

  while (segmentNo < itinerary.size() && itinerary[segmentNo]->destination()->loc() != _city2)
    ++segmentNo;

  if (segmentNo < itinerary.size())
  {
    retval.valid = PASS;
    for (size_t i = begSeg; i <= segmentNo; ++i)
      retval->matchedSegments[i] = true;
  }

  return retval;
}

////////////////////////

std::string
FromTo::toString(int level) const
{
  std::string s = printTabs(level);
  s += "Travel from or to " + _city + "\n";
  return s;
}

PredicateReturnType
FromTo::test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx)
{
  PredicateReturnType retval;
  /*AirSeg* segment = dynamic_cast<AirSeg*>(itinerary[0].get());
  AirSeg* nextToLast
      = dynamic_cast<AirSeg*>(itinerary[itinerary.size() - 1].get());
  if (!segment || !nextToLast) return retval;
  */

  if (itinerary[0]->origin()->loc() == _city ||
      itinerary[itinerary.size() - 1]->destination()->loc() == _city)
    retval.valid = PASS;

  return retval;
}

///////////////

std::string
OnlyHiddenStops::toString(int level) const
{
  std::string s = printTabs(level);
  s += "Selecting hidden points only\n";
  return s;
}

PredicateReturnType
OnlyHiddenStops::test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx)
{
  PredicateReturnType retval;
  retval.valid = PASS;
  /// @todo hidden stops
  /*
  std::vector<TravelSeg*>& itin
      = const_cast<std::vector<TravelSeg*>&>(itinerary);

  std::vector<TravelSeg*>::iterator it;
  for(it = itin.begin(); it != itin.end(); ++it)
      if (it->hiddenStop)
          itin.erase(it);
  */
  return retval;
}

///////////////

std::string
NoHiddenStops::toString(int level) const
{
  std::string s = printTabs(level);
  s += "No hidden points\n";
  return s;
}

PredicateReturnType
NoHiddenStops::test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx)
{
  PredicateReturnType retval;
  retval.valid = PASS;
  /// @todo hidden stops
  /*
  std::vector<TravelSeg*>& itin
      = const_cast<std::vector<TravelSeg*>&>(itinerary);

  std::vector<TravelSeg*>::iterator it;
  for(it = itin.begin(); it != itin.end(); ++it)
      if (!it->hiddenStop)
          itin.erase(it);
  */
  return retval;
}

///////////////////////

std::string
IsEquipment::toString(int level) const
{
  std::string s = printTabs(level);
  s += "IsEquipment: " + _equip + "\n";
  return s;
}

PredicateReturnType
IsEquipment::test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx)
{
  PredicateReturnType retval;
  PredicateOutputPrinter p(retval.valid, "IsEquipment");
  retval->matchedSegments.assign(itinerary.size(), false);
  const NoPNRPricingTrx* WQTrx = dynamic_cast<const NoPNRPricingTrx*>(&trx);
  bool isWQTrx = (WQTrx != nullptr);

  for (size_t segmentNo = 0; segmentNo < itinerary.size(); ++segmentNo)
  {
    if (UNLIKELY(isWQTrx))
    {

      AirSeg* segment = nullptr;
      segment = dynamic_cast<AirSeg*>(itinerary[segmentNo]);
      if (segment && segment->flightNumber() != 0 && !itinerary[segmentNo]->equipmentType().empty())
      {
        if ((itinerary[segmentNo]->equipmentType() == _equip) ||
            (_equip == A340_ALL_SERIES &&
             !strncmp(itinerary[segmentNo]->equipmentType().c_str(), A340_ALL_SERIES.c_str(), 2)))
        {
          retval.valid = PASS;
          retval->matchedSegments[segmentNo] = true;
        }
      }
      else if (itinerary[segmentNo]->segmentType() == Open)
      {
        retval.valid = SOFTPASS;
        retval->matchedSegments[segmentNo] = true;
        notify();
      }
    }
    else if (UNLIKELY(itinerary[segmentNo]->segmentType() == Open))
    {
      if (retval.valid != PASS)
        retval.valid = SOFTPASS;
      retval->matchedSegments[segmentNo] = true;
    }
    else if ((itinerary[segmentNo]->equipmentType() == _equip) ||
             (_equip == A340_ALL_SERIES &&
              !strncmp(itinerary[segmentNo]->equipmentType().c_str(), A340_ALL_SERIES.c_str(), 2)))
    {
      retval.valid = PASS;
      retval->matchedSegments[segmentNo] = true;
      // retval->matchedSegments.push_back(segmentNo);
    }
  }

  return retval;
}

/////////////////

std::string
NonStopFlight::toString(int level) const
{
  std::string s = printTabs(level);
  s += "Is flight a Non-stop flight?\n";
  return s;
}

PredicateReturnType
NonStopFlight::test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx)
{
  PredicateReturnType retval;
  PredicateOutputPrinter p(retval.valid, "NonStopFlight");
  retval->matchedSegments.assign(itinerary.size(), false);

  if (itinerary.size() == 1)
  {
    if (!_countHiddenStop || itinerary.front()->hiddenStops().empty())
    {
      retval.valid = PASS;
      retval->matchedSegments[0] = true;
    }
  }

  return retval;
}

/////////////////

std::string
DirectFlight::toString(int level) const
{
  std::string s = printTabs(level);
  s += "Is flight a direct flight?\n";
  return s;
}

PredicateReturnType
DirectFlight::test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx)
{
  SameFlightNumber sf;
  PredicateReturnType retval = sf.test(itinerary, trx);
  PredicateOutputPrinter p(retval.valid, "DirectFlight");

  if (retval.valid == FAIL)
    return retval;

  /// @todo are there any stopovers?

  return retval;
}

/////////////////

std::string
OnlineConnection::toString(int level) const
{
  std::string s = printTabs(level);
  s += "Is itinerary an online connection?\n";
  return s;
}

PredicateReturnType
OnlineConnection::test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx)
{
  PredicateReturnType retval;
  PredicateOutputPrinter p(retval.valid, "OnlineConnection");
  retval->matchedSegments.assign(itinerary.size(), false);

  if (itinerary.size() < 2) // need at least 2 segments
    return retval;

  std::vector<TravelSeg*>::const_iterator it;
  AirSeg* segment = dynamic_cast<AirSeg*>(itinerary[0]);
  if (!segment)
    return retval;

  CarrierCode cxr = segment->carrier();
  retval.valid = PASS;

  PredicateReturnType::iterator retitr;
  for (it = itinerary.begin(), retitr = retval->matchedSegments.begin(); it != itinerary.end();
       ++it, ++retitr)
  {
    segment = dynamic_cast<AirSeg*>(*it);
    if (!segment || segment->carrier() != cxr)
    {
      retval.valid = FAIL;
      return retval;
    }
    else
      *retitr = true;
  }

  return retval;
}

/////////////////

std::string
InterlineConnection::toString(int level) const
{
  std::string s = printTabs(level);
  s += "Is itinerary an interline connection?\n";
  return s;
}

PredicateReturnType
InterlineConnection::test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx)
{
  PredicateReturnType retval;
  PredicateOutputPrinter p(retval.valid, "InterlineConnection");
  retval->matchedSegments.assign(itinerary.size(), false);

  if (itinerary.size() < 2) // need at least 2 segments
    return retval;

  AirSeg* segment = dynamic_cast<AirSeg*>(itinerary[0]);
  if (!segment)
    return retval;

  std::vector<TravelSeg*>::const_iterator it;
  CarrierCode cxr = segment->carrier();
  retval.valid = underNegAppl() ? FAIL : PASS;
  retval->matchedSegments[0] = !underNegAppl();

  PredicateReturnType::iterator retitr;
  for (it = itinerary.begin(), ++it, retitr = retval->matchedSegments.begin(), ++retitr;
       it != itinerary.end();
       ++it, ++retitr)
  {
    segment = dynamic_cast<AirSeg*>(*it);
    if (!underNegAppl())
    {
      if (!segment || segment->carrier() == cxr)
      {
        retval.valid = FAIL;
        retval->matchedSegments[0] = false;
        return retval;
      }
      *retitr = true;
    }
    else
    {
      *retitr = segment && segment->carrier() != cxr;
      if (*retitr)
        retval.valid = PASS;
    }
    if (segment)
      cxr = segment->carrier();
  }

  return retval;
}

/////////////////

std::string
SameFlightNumber::toString(int level) const
{
  std::string s = printTabs(level);
  s += "All segments have the same flight number?\n";
  return s;
}

PredicateReturnType
SameFlightNumber::test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx)
{
  PredicateReturnType retval;
  PredicateOutputPrinter p(retval.valid, "SameFlightNumber");
  AirSeg* segment = dynamic_cast<AirSeg*>(itinerary[0]);
  std::vector<TravelSeg*>::const_iterator it;
  retval->matchedSegments.assign(itinerary.size(), false);

  if (UNLIKELY(!segment))
    return retval;

  CarrierCode cxr = segment->carrier();
  int flightNo = segment->flightNumber();
  retval.valid = PASS;

  PredicateReturnType::iterator retitr;
  for (it = itinerary.begin(), retitr = retval->matchedSegments.begin(); it != itinerary.end();
       ++it, ++retitr)
  {
    segment = dynamic_cast<AirSeg*>(*it);
    if (!segment || segment->carrier() != cxr || segment->flightNumber() != flightNo)
    {
      retval.valid = FAIL;
    }
    else
    {
      *retitr = true;
    }
  }

  return retval;
}

} // namespace tse
