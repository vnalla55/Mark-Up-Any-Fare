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

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

#include <memory>
#include <set>
#include <vector>

namespace tse
{
class RexBaseTrx;
class ProcessTagPermutation;
class Diag689Collector;
class FarePath;
class TravelSeg;

class StopoverConnectionValidator
{
  friend class StopoverConnectionValidatorTest;

public:
  StopoverConnectionValidator(const RexBaseTrx& trx,
                              const FarePath& farePath,
                              Diag689Collector* dc)
    : _trx(trx),
      _farePath(farePath),
      _diag(dc),
      _statusCache(new ValidationStatusCache)
  {
    init();
  }


  void setDiagnostic(Diag689Collector* dc) { _diag = dc; }

  bool match(const ProcessTagPermutation& perm) const;

private:
  struct ValidationStatusCache
  {
    struct Status
    {
      Status() : isDetermined(false), value(false) {}
      bool determine(bool state)
      {
        isDetermined = true;
        return value = state;
      }
      bool isDetermined, value;
    } connections, stopovers, both;
  };

  typedef std::multiset<LocCode> LocationsSet;
  typedef std::vector<TravelSeg*> TravelSegmentsVec;

  bool isDiagnostic() const;
  void init();
  bool matchImpl(const Indicator& byte,
                 LocationsSet& unmatchedStopOvers,
                 LocationsSet& unmatchedConnections) const;
  void getSegmentsWithinFareComponents(const FarePath& farePath,
                                       TravelSegmentsVec& segs) const;

  const RexBaseTrx& _trx;
  const FarePath& _farePath;
  Diag689Collector* _diag;
  TravelSegmentsVec _excSegs, _newSegs;
  std::unique_ptr<ValidationStatusCache> _statusCache;
};
} // tse
