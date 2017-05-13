#pragma once

#include "Common/CarrierUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/TravelSeg.h"
#include "Rules/CommonPredicates.h"
#include "Rules/Debug.h"
#include "Rules/FlightApplication.h"


#include <bitset>
#include <deque>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace tse
{

class PredicateObserver;

class PredicateNotifier
{
  PredicateObserver* _observer = nullptr;

public:
  void notify()
  {
    if (_observer)
      _observer->getNotified();
  }
  void init(PredicateObserver& observer) { _observer = &observer; }
};

class CxrExactMatch
{
public:
  bool operator()(const CarrierCode& cxr1, const CarrierCode& cxr2, const PricingTrx& trx)
  {
    return cxr1 == cxr2;
  }
};

class CxrAllianceMatch
{
public:
  bool operator()(const CarrierCode& cxr, const CarrierCode& alliance, const PricingTrx& trx)
  {
    if (cxr == alliance)
      return true;

    return CarrierUtil::carrierAllianceMatch(cxr, alliance, trx);
  }
};

/// Tests if which segments of itinerary match a given flight cxr/number
template <typename T>
class IsFlight : public Predicate
{
public:
  IsFlight() : Predicate("IsFlight") {}

  /** @param cxr Carrier
   *  @param flight Flight number
   */
  void initialize(const CarrierCode& cxr, const CarrierCode& optcxr, const unsigned flight)
  {
    _cxr = cxr;
    _optcxr = optcxr, _flight = flight;
  }

  inline virtual std::string toString(int level = 0) const override;

  inline virtual PredicateReturnType
  test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx) override;

  static const std::string A340_ALL_SERIES;

private:
  inline bool optCarrierCheck(const AirSeg* segment, const PricingTrx& trx);
  inline bool flightCheck(const AirSeg* segment);

  CarrierCode _cxr;
  CarrierCode _optcxr;
  int _flight = 0;
  T _comparator;
};

/// Tests if which segments of itinerary fall in a given range of flight numbers
template <typename T>
class IsFlightBetween : public Predicate
{
public:
  IsFlightBetween() : Predicate("IsFlightBetween") {}

  /** @param cxr Carrier
   *  @param fl Lower bound of flight number
   *  @param fh Higher bound of flight number
   */
  void initialize(const CarrierCode& cxr, const CarrierCode& optcxr, int fl, int fh)
  {
    if (fl > fh)
    {
      _flightLow = fh;
      _flightHigh = fl;
    }
    else
    {
      _flightLow = fl;
      _flightHigh = fh;
    }
    _cxr = cxr;
    _optcxr = optcxr;
  }

  inline virtual std::string toString(int level = 0) const override;

  inline virtual PredicateReturnType
  test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx) override;

private:
  inline bool optCarrierCheck(const AirSeg* segment, const PricingTrx& trx);
  inline bool flightCheck(const unsigned segFlight);

  unsigned _flightLow = 0;
  unsigned _flightHigh = 0;
  CarrierCode _cxr;
  CarrierCode _optcxr;
  T _comparator;
};

/// Tests if both flights condiotions are present in an itinerary
/// (order does not matter).
class AndFlight : public Predicate
{
public:
  AndFlight() : Predicate("AndFlight") {}

  void initialize(Predicate* cond1, Predicate* cond2)
  {
    _cond1 = cond1;
    _cond2 = cond2;
  }

  virtual std::string toString(int level = 0) const override;

  virtual PredicateReturnType
  test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx) override;

private:
  Predicate* _cond1 = nullptr;
  Predicate* _cond2 = nullptr;
};

/// Tests some blablabla
class AndConnectingFlights : public Predicate
{
public:
  AndConnectingFlights() : Predicate("ConnectingFlights") {}

  void initialize(Predicate* and1,
                  Predicate* and2,
                  Predicate* conn,
                  bool allConnecting,
                  bool andFirst = true,
                  bool negAppl = false);

  virtual std::string toString(int level = 0) const override;

  virtual PredicateReturnType
  test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx) override;

private:
  Predicate* _and1 = nullptr;
  Predicate* _and2 = nullptr;
  Predicate* _conn = nullptr;
  bool _allConnecting = false;
  bool _andFirst = true;
  bool _negAppl = false;
};

/// Tests if all flight is itinerary satisfy condition it holds
class AllFlightsMatch : public Predicate
{
public:
  AllFlightsMatch() : Predicate("AllFlightsMatch"), _pred(nullptr) {};

  /** @param pred Predicate to test
   */
  void initialize(Predicate* pred) { _pred = pred; }

  virtual std::string toString(int level = 0) const override;

  virtual PredicateReturnType
  test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx) override;

private:
  Predicate* _pred;
};

class DayOfWeekPredicate : public Predicate
{
public:
  enum DayName
  {
    Mon = 1,
    Tue,
    Wed,
    Thu,
    Fri,
    Sat,
    Sun
  };

  DayOfWeekPredicate() : Predicate("DayOfWeek") {};

  void initialize(const std::set<DayName>& days);

  virtual std::string toString(int level = 0) const override;

  virtual PredicateReturnType
  test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx) override;

private:
  /// Converts day of week number from boost convention to ATPco convention
  ///  (thus predicate convention).
  DayName boostToAtpco(unsigned short day) const;

  std::bitset<8> _days;
};

class Via : public Predicate
{
public:
  Via() : Predicate("Via") {}

  void initialize(LocCode& city) { _city = city; }

  virtual std::string toString(int level = 0) const override;
  virtual PredicateReturnType
  test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx) override;

private:
  LocCode _city;
};

class BetweenCities : public Predicate
{
public:
  BetweenCities() : Predicate("BetweenCities") {}

  void initialize(LocCode& city1, LocCode& city2)
  {
    _city1 = city1;
    _city2 = city2;
  }

  virtual std::string toString(int level = 0) const override;
  virtual PredicateReturnType
  test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx) override;

private:
  LocCode _city1;
  LocCode _city2;
};

/// Travel from or to the given city
class FromTo : public Predicate
{
public:
  FromTo() : Predicate("FromTo") {}

  void initialize(const LocCode& city) { _city = city; }

  virtual std::string toString(int level = 0) const override;
  virtual PredicateReturnType
  test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx) override;

private:
  LocCode _city;
};

// Does not validate - only extracts point from itinerary
class OnlyHiddenStops : public Predicate
{
public:
  OnlyHiddenStops() : Predicate("OnlyHiddenStops") {}

  virtual std::string toString(int level = 0) const override;
  virtual PredicateReturnType
  test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx) override;
};

// Does not validate - only extracts point from itinerary
class NoHiddenStops : public Predicate
{
public:
  NoHiddenStops() : Predicate("NoHiddenStops") {}
  virtual std::string toString(int level = 0) const override;
  virtual PredicateReturnType
  test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx) override;
};

class IsEquipment : public Predicate, public PredicateNotifier
{
public:
  IsEquipment() : Predicate("IsEquipment") {}

  void initialize(const std::string& equip) { _equip = equip; }

  virtual std::string toString(int level = 0) const override;
  virtual PredicateReturnType
  test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx) override;

private:
  std::string _equip;
};

class NonStopFlight : public Predicate
{
public:
  NonStopFlight() : Predicate("NonStopFlight") {}

  virtual std::string toString(int level = 0) const override;
  virtual PredicateReturnType
  test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx) override;

  bool& countHiddenStop() { return _countHiddenStop; }

private:
  bool _countHiddenStop = true;
};

class DirectFlight : public Predicate
{
public:
  DirectFlight() : Predicate("DirectFlight") {}

  virtual std::string toString(int level = 0) const override;
  virtual PredicateReturnType
  test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx) override;
};

class SameFlightNumber : public Predicate
{
public:
  SameFlightNumber() : Predicate("SameFlightNumber") {}

  virtual std::string toString(int level = 0) const override;
  virtual PredicateReturnType
  test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx) override;
};

template <unsigned minStops, unsigned maxStops = 0>
class MultiStopFlightTemp : public Predicate
{
public:
  MultiStopFlightTemp() : Predicate("MultiStopFlightTemp") {}

  virtual std::string toString(int level = 0) const override;
  virtual PredicateReturnType
  test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx) override;
  bool& countHiddenStop() { return _countHiddenStop; }

private:
  unsigned _minStops = minStops;
  unsigned _maxStops = maxStops;
  bool _countHiddenStop = true;
};

template <unsigned minStops, unsigned maxStops>
std::string
MultiStopFlightTemp<minStops, maxStops>::toString(int level) const
{
  std::string s = printTabs(level);
  std::stringstream temp;
  temp << _minStops;
  s += "Is flight a multi stop flight with " + temp.str() + " stops?\n";
  return s;
}

template <unsigned minStops, unsigned maxStops>
PredicateReturnType
MultiStopFlightTemp<minStops, maxStops>::test(const std::vector<TravelSeg*>& itinerary,
                                              const PricingTrx& trx)
{
  PredicateReturnType retval;
  PredicateOutputPrinter p(retval.valid, "MultiStopFlightTemp");

  uint16_t numOfStops = itinerary.size() - 1;
  if (_countHiddenStop)
  {
    std::vector<TravelSeg*>::const_iterator tvlSegI = itinerary.begin();
    const std::vector<TravelSeg*>::const_iterator tvlSegIEnd = itinerary.end();
    for (; tvlSegI != tvlSegIEnd; tvlSegI++)
    {
      numOfStops += (*tvlSegI)->hiddenStops().size();
    }
  }

  if (numOfStops < _minStops)
    return retval;
  if (_maxStops != 0 && numOfStops > _maxStops)
    return retval;

  // ATPco definitions make this equivalents
  SameFlightNumber sf;
  return sf.test(itinerary, trx);
}

using OneStopFlight = MultiStopFlightTemp<1, 1>;
using MultiStopFlight = MultiStopFlightTemp<2>;

class OnlineConnection : public Predicate
{
public:
  OnlineConnection() : Predicate("OnlineConnection") {}

  virtual std::string toString(int level = 0) const override;
  virtual PredicateReturnType
  test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx) override;
};

class InterlineConnection : public Predicate
{
public:
  InterlineConnection() : Predicate("InterlineConnection") {}

  virtual std::string toString(int level = 0) const override;
  virtual PredicateReturnType
  test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx) override;
  bool underNegAppl() const { return _underNegAppl; }
  bool& underNegAppl() { return _underNegAppl; }

private:
  bool _underNegAppl = false;
};

template <typename T>
std::string
IsFlight<T>::toString(int level) const
{
  std::string s = printTabs(level);
  std::stringstream temp;
  temp << _flight;
  s += _name + " (" + _cxr + " " + temp.str() + ")\n";
  return s;
}

template <typename T>
PredicateReturnType
IsFlight<T>::test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx)
{
  PredicateReturnType retval;
  PredicateOutputPrinter p(retval.valid, "IsFlight");
  AirSeg* segment;
  retval->matchedSegments.assign(itinerary.size(), false);

  for (size_t segmentNo = 0; segmentNo < itinerary.size(); ++segmentNo)
  {
    segment = dynamic_cast<AirSeg*>(itinerary[segmentNo]);
    if (itinerary[segmentNo]->segmentType() == Open)
    {
      bool cxrMatched = false;
      bool opCxrMatched = false;
      bool fltMatched = false;

      if (!segment)
      {
        continue; // should never happen, fail this segment
      }
      if (segment->carrier() != FlightApplication::ANY_CARRIER)
      {
        if (!_comparator(segment->carrier(), _cxr, trx))
        {
          continue; // not open carrier we can have hard fail
        }
        else
        {
          cxrMatched = true;
        }
      }

      if (_optcxr.empty())
      {
        opCxrMatched = true;
      }
      else
      {
        if (segment->operatingCarrierCode() != FlightApplication::ANY_CARRIER)
        {
          if (!_comparator(segment->operatingCarrierCode(), _optcxr, trx))
          {
            continue; // hard fail
          }
          else
          {
            opCxrMatched = true;
          }
        }
      }

      if (segment->flightNumber() != 0)
      {
        if (cxrMatched && opCxrMatched && flightCheck(segment))
        {
          retval->matchedSegments[segmentNo] = true;
          retval.valid = PASS;
        }
      }
      else
      {
        if (_flight == RuleConst::ANY_FLIGHT)
        {
          fltMatched = true;
        }
        retval->matchedSegments[segmentNo] = true;
        if (cxrMatched && opCxrMatched && fltMatched)
        {
          retval.valid = PASS;
        }
        else if (retval.valid != PASS)
        {
          retval.valid = SOFTPASS;
        }
      }
      continue; // segmentType == OPEN
    }

    if (UNLIKELY(itinerary[segmentNo]->segmentType() == Surface))
    {
      retval->matchedSegments[segmentNo] = true;
      continue;
    }

    if (segment && _comparator(segment->carrier(), _cxr, trx) && optCarrierCheck(segment, trx) &&
        flightCheck(segment))
    {
      retval.valid = PASS;
      retval->matchedSegments[segmentNo] = true;
    }
  }

  return retval;
}

template <typename T>
bool
IsFlight<T>::optCarrierCheck(const AirSeg* segment, const PricingTrx& trx)
{
  return _optcxr.empty() || _optcxr == EMPTY_CARRIER ||
         _comparator(segment->operatingCarrierCode(), _optcxr, trx);
}

template <typename T>
bool
IsFlight<T>::flightCheck(const AirSeg* segment)
{
  return _flight == RuleConst::ANY_FLIGHT || static_cast<int>(segment->flightNumber()) == _flight;
}

////////////////////////////

template <typename T>
std::string
IsFlightBetween<T>::toString(int level) const
{
  std::string s = printTabs(level);
  std::stringstream temp;
  temp << _flightLow << " - " << _flightHigh;
  s += _name + " (" + _cxr + " " + temp.str() + ")\n";
  return s;
}

template <typename T>
PredicateReturnType
IsFlightBetween<T>::test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx)
{
  PredicateReturnType retval;
  PredicateOutputPrinter p(retval.valid, "IsFlightBetween");
  AirSeg* segment;
  retval->matchedSegments.assign(itinerary.size(), false);

  for (size_t segmentNo = 0; segmentNo < itinerary.size(); ++segmentNo)
  {
    segment = dynamic_cast<AirSeg*>(itinerary[segmentNo]);
    if (UNLIKELY(!segment))
      continue;

    if (segment->segmentType() == Open)
    {
      if (segment->carrier() != FlightApplication::ANY_CARRIER &&
          !_comparator(segment->carrier(), _cxr, trx))
      {
        continue; // not open carrier we can have hard fail
      }

      if (!_optcxr.empty() && (segment->operatingCarrierCode() != FlightApplication::ANY_CARRIER) &&
          !_comparator(segment->operatingCarrierCode(), _optcxr, trx))
      {
        continue; // hard fail
      }

      if (segment->flightNumber() != 0)
      {
        if (flightCheck(segment->flightNumber()))
        {
          retval.valid = PASS;
          retval->matchedSegments[segmentNo] = true;
        }
      }
      else
      {
        if (retval.valid != PASS)
        {
          retval.valid = SOFTPASS;
        }
        retval->matchedSegments[segmentNo] = true;
      }
      continue;

    } // segmentType == OPEN

    if (_comparator(segment->carrier(), _cxr, trx) && optCarrierCheck(segment, trx) &&
        flightCheck(segment->flightNumber()))
    {
      retval.valid = PASS;
      retval->matchedSegments[segmentNo] = true;
      // retval->matchedSegments.push_back(segmentNo);
    }
  }

  return retval;
}

template <typename T>
bool
IsFlightBetween<T>::optCarrierCheck(const AirSeg* segment, const PricingTrx& trx)
{
  return _optcxr.empty() || _optcxr == EMPTY_CARRIER ||
         _comparator(segment->operatingCarrierCode(), _optcxr, trx);
}

template <typename T>
bool
IsFlightBetween<T>::flightCheck(const unsigned segFlight)
{
  return segFlight >= _flightLow && segFlight <= _flightHigh;
}
} // namespace tse
