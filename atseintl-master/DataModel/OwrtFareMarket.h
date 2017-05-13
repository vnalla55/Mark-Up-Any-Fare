// ----------------------------------------------------------------
//
//   Copyright Sabre 2011
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#pragma once

#include "Common/LocUtil.h"
#include "Common/ShpqTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/FareMarket.h"
#include "DataModel/ItinIndex.h"
#include "DataModel/OwrtFmPatternSummary.h"
#include "DBAccess/Loc.h"

#include <memory>
#include <sstream>
#include <vector>

namespace tse
{

class PaxTypeFare;
class Loc;
class Trx;
class Fare;

namespace shpq
{

class OwrtFareMarket;
typedef std::shared_ptr<OwrtFareMarket> OwrtFareMarketPtr;

class LocationPair
{
protected:
  const Loc* _location;
  const LocCode& _multiCity;

  bool cmp(const LocationPair& other) const
  {
    if (_location->loc() != other.getLocation()->loc())
      return _location->loc() < other.getLocation()->loc();
    if (_multiCity != other.getMultiCity())
      return _multiCity < other.getMultiCity();

    return false;
  }

public:
  LocationPair(const Loc* location, const LocCode& multiCity)
    : _location(location), _multiCity(multiCity)
  {
  }

  bool operator<(const LocationPair& other) const { return cmp(other); }

  bool operator==(const LocationPair& other) const
  {
    return LocUtil::isSamePoint(*_location, _multiCity, *other.getLocation(), other.getMultiCity());
  }

  const Loc* getLocation() const { return _location; }
  const LocCode& getMultiCity() const { return _multiCity; }
  std::string toString() const
  {
    std::ostringstream oss;
    oss << _location->loc() << " [" << _multiCity << "]";

    return oss.str();
  }
};

class MultiAirportLocationPair : public LocationPair
{
private:
  bool _isConnectionCity;

public:
  MultiAirportLocationPair(const Loc* location,
                           const LocCode& multiCity,
                           const bool isConnectionCity)
    : LocationPair(location, multiCity), _isConnectionCity(isConnectionCity)
  {
  }

  bool operator<(const MultiAirportLocationPair& other) const
  {
    if (_isConnectionCity || other._isConnectionCity)
      return _multiCity < other.getMultiCity();
    return cmp(other);
  }

  bool operator==(const MultiAirportLocationPair& other) const
  {
    if (_isConnectionCity || other._isConnectionCity)
      return _multiCity == other.getMultiCity();

    return LocUtil::isSamePoint(*_location, _multiCity, *other.getLocation(), other.getMultiCity());
  }
};

class OwrtFareMarket
{
public:
  typedef std::vector<PaxTypeFare*> PaxTypeFareVector;
  typedef PaxTypeFareVector::iterator iterator;
  typedef PaxTypeFareVector::const_iterator const_iterator;

  static OwrtFareMarketPtr create(Trx&, SolutionType, FareMarket*, ItinIndex::Key);

  // use create() instead of constructor
  OwrtFareMarket(SolutionType solutionType, FareMarket* fm, ItinIndex::Key cxrIndexKey)
    : _fareMarket(fm),
      _cxrIndexKey(cxrIndexKey),
      _origin(fm->origin(), fm->boardMultiCity()),
      _destination(fm->destination(), fm->offMultiCity()),
      _solutionType(solutionType),
      _hasNormal(false),
      _hasSpecial(false),
      _hasTag1(false),
      _hasTag2(false),
      _hasTag3(false)
  {
    if (_solutionType == OW)
      insertFares(fm->ow_begin(), fm->ow_end());
    else if (LIKELY(_solutionType == HRT))
      insertFares(fm->hrt_begin(), fm->hrt_end());
    analyzeValidFares();
  }

  iterator begin() { return _paxTypeFares.begin(); }
  iterator end() { return _paxTypeFares.end(); }

  const_iterator begin() const { return _paxTypeFares.begin(); }
  const_iterator end() const { return _paxTypeFares.end(); }

  FareMarket* getFareMarket() const { return _fareMarket; }

  MoneyAmount lowerBound() const;

  const LocationPair& getOriginLocation() const { return _origin; }
  const LocationPair& getDestinationLocation() const { return _destination; }

  const Fare* getLowerBoundFare() const;

  const CarrierCode& getGoverningCarrier() const { return _fareMarket->governingCarrier(); }

  SolutionType getType() const { return _solutionType; }

  OwrtFmPatternSummary& getMutableSolPatternSummary() { return _solPatternSummary; }
  const OwrtFmPatternSummary& getSolPatternSummary() const { return _solPatternSummary; }

  bool hasNormal() { return _hasNormal; }
  bool hasSpecial() { return _hasSpecial; }
  bool hasTag1() { return _hasTag1; }
  bool hasTag2() { return _hasTag2; }
  bool hasTag3() { return _hasTag3; }

  bool empty() const { return _paxTypeFares.empty(); }
  void analyzeValidFares();

  std::string str() const;

protected:
  template <class Iterator>
  void insertFares(Iterator begin, Iterator end)
  {
    for (Iterator it = begin; it != end; ++it)
      _paxTypeFares.push_back(*it);
  }

private:
  PaxTypeFareVector _paxTypeFares;
  FareMarket* _fareMarket;
  ItinIndex::Key _cxrIndexKey;

  LocationPair _origin;
  LocationPair _destination;

  OwrtFmPatternSummary _solPatternSummary;

  SolutionType _solutionType;
  bool _hasNormal;
  bool _hasSpecial;
  bool _hasTag1;
  bool _hasTag2;
  bool _hasTag3;
};
}
} // namespace tse::shpq

