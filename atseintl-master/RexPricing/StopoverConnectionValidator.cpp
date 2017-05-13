//----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "RexPricing/StopoverConnectionValidator.h"

#include "DataModel/ExcItin.h"
#include "DataModel/FarePath.h"
#include "DataModel/ProcessTagPermutation.h"
#include "DataModel/RexBaseTrx.h"
#include "DataModel/RexPricingOptions.h"
#include "Diagnostic/Diag689Collector.h"

namespace tse
{

namespace
{
typedef std::multiset<LocCode> LocationsSet;
typedef std::vector<TravelSeg*> TravelSegmentsVec;

class InsertMulticity : public std::insert_iterator<LocationsSet>
{
private:
  struct Proxy
  {
    void operator=(const TravelSeg* seg) { s->insert(seg->offMultiCity()); }
    LocationsSet* s;
  } proxy;

public:
  InsertMulticity(LocationsSet& s)
    : std::insert_iterator<LocationsSet>(s, s.begin())
  {
    proxy.s = &s;
  }

  Proxy& operator*() { return proxy; }
};

struct IsStopover : public std::unary_function<const TravelSeg*, bool>
{
  bool operator()(const TravelSeg* seg) const
  {
    return seg->isForcedStopOver() || seg->stopOver();
  }
};

struct IsConnection : public std::unary_function<const TravelSeg*, bool>
{
  bool operator()(const TravelSeg* seg) const
  {
    return seg->isForcedConx() || !seg->stopOver();
  }
};

template <typename P>
void
getLocations(const TravelSegmentsVec& segs,
             const P& predicat, LocationsSet& locs)
{
  std::remove_copy_if(segs.begin(), segs.end(), InsertMulticity(locs), std::not1(predicat));
}

template <typename P>
bool
haveSameLocations(const TravelSegmentsVec& first,
                  const TravelSegmentsVec& second,
                  const P& predicat,
                  LocationsSet& unmatched)
{
  LocationsSet firstLocs, secondLocs;
  getLocations(first, predicat, firstLocs);
  getLocations(second, predicat, secondLocs);
  std::set_symmetric_difference(firstLocs.begin(),
                                firstLocs.end(),
                                secondLocs.begin(),
                                secondLocs.end(),
                                std::inserter(unmatched, unmatched.begin()));
  return unmatched.empty();
}

} // namespace

bool
StopoverConnectionValidator::isDiagnostic() const
{
  return _diag && _diag->filterPassed();
}

void
StopoverConnectionValidator::init()
{
  getSegmentsWithinFareComponents(*_trx.exchangeItin().front()->farePath().front(), _excSegs);
  getSegmentsWithinFareComponents(_farePath, _newSegs);
}

bool
StopoverConnectionValidator::match(const ProcessTagPermutation& perm) const
{
  const Indicator byte = perm.getStopoverConnectionByte();
  LocationsSet unmatchedStopOvers, unmatchedConnections;

  if (!isDiagnostic())
    return matchImpl(byte, unmatchedStopOvers, unmatchedConnections);

  *_diag << "STOP/CONX CHECK: " << byte << "\n";

  if (matchImpl(byte, unmatchedStopOvers, unmatchedConnections))
    return true;

  if (!unmatchedConnections.empty())
  {
    *_diag << "  UNMATCHED CONX: ";
    std::copy(unmatchedConnections.begin(),
              unmatchedConnections.end(),
              std::ostream_iterator<LocCode>(*_diag, " "));
    *_diag << "\n";
  }

  if (!unmatchedStopOvers.empty())
  {
    *_diag << "  UNMATCHED STPS: ";
    std::copy(unmatchedStopOvers.begin(),
              unmatchedStopOvers.end(),
              std::ostream_iterator<LocCode>(*_diag, " "));
    *_diag << "\n";
  }

  return false;
}

void
StopoverConnectionValidator::getSegmentsWithinFareComponents(const FarePath& farePath,
                                                             TravelSegmentsVec& segs) const
{
  for (const PricingUnit* pu : farePath.pricingUnit())
  {
    for (const FareUsage* fu : pu->fareUsage())
    {
      std::copy(fu->travelSeg().begin(), fu->travelSeg().end() - 1, std::back_inserter(segs));
    }
  }
}

bool
StopoverConnectionValidator::matchImpl(const Indicator& byte,
                                       LocationsSet& unmatchedStopovers,
                                       LocationsSet& unmatchedConnections) const
{
  switch (byte)
  {
  case ProcessTagPermutation::STOPCONN_BLANK:
    return true;

  case ProcessTagPermutation::STOPCONN_B:
  {
    ValidationStatusCache::Status& status = _statusCache->both;
    return status.isDetermined ? status.value
        : status.determine(haveSameLocations(_excSegs, _newSegs,
                                             IsConnection(), unmatchedConnections) &&
                           haveSameLocations(_excSegs, _newSegs,
                                             IsStopover(), unmatchedStopovers));
  }

  case ProcessTagPermutation::STOPCONN_C:
  {
    ValidationStatusCache::Status& status = _statusCache->connections;
        return status.isDetermined ? status.value
          : status.determine(haveSameLocations(_excSegs, _newSegs,
                                               IsConnection(), unmatchedConnections));
  }

  case ProcessTagPermutation::STOPCONN_S:
  {
    ValidationStatusCache::Status& status = _statusCache->stopovers;
    return status.isDetermined ? status.value
        : status.determine(haveSameLocations(_excSegs, _newSegs,
                                             IsStopover(), unmatchedStopovers));
  }
  default:
    ;
  }
  return false;
}

} // tse
