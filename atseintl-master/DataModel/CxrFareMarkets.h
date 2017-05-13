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

#include "Common/ShpqTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/OwrtFareMarket.h"

#include <memory>
#include <vector>

namespace tse
{

class Fare;
class Loc;
class Trx;

namespace shpq
{

class OwrtFareMarket;
class CxrFareMarkets;
typedef std::shared_ptr<CxrFareMarkets> CxrFareMarketsPtr;

class CxrFareMarkets
{
public:
  typedef std::shared_ptr<OwrtFareMarket> OwrtFareMarketPtr;
  typedef std::vector<OwrtFareMarketPtr> OwrtFareMarketVector;
  typedef OwrtFareMarketVector::iterator iterator;
  typedef OwrtFareMarketVector::const_iterator const_iterator;

  static CxrFareMarketsPtr create(Trx&, SolutionType);

  // use create() instead of constructor
  explicit CxrFareMarkets(SolutionType solutionType) : _solutionType(solutionType) {}

  void insert(OwrtFareMarketPtr);
  iterator begin() { return _owrtFareMarkets.begin(); }
  iterator end() { return _owrtFareMarkets.end(); }

  const_iterator begin() const { return _owrtFareMarkets.begin(); }
  const_iterator end() const { return _owrtFareMarkets.end(); }

  MoneyAmount lowerBound() const;
  SolutionType getType() const { return _solutionType; }

  const LocationPair& getOriginLocation() const;
  const LocationPair& getDestinationLocation() const;
  const Fare* getLowerBoundFare() const;

private:
  OwrtFareMarketVector _owrtFareMarkets;
  SolutionType _solutionType; // OW / RT
};
}
} // namespace tse::shpq

