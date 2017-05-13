//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.

//-------------------------------------------------------------------

#pragma once

#include <vector>
#include <set>
#include <boost/cstdint.hpp>

namespace tse
{
struct CacheMemoryDiagnostic
{
  CacheMemoryDiagnostic()
    : _cacheSize(0)
    , _uncompressedSize(0)
    , _objectsInAccumulator(0)
    , _totalObjects(0)
    , _objectsInUncompressed(0)
  {
  }
  std::set<boost::int64_t> _activeTrx;
  struct CacheDeleterUnitDiagnostic
  {
    CacheDeleterUnitDiagnostic()
      : _numberObjects(0)
    {
    }
    std::set<boost::int64_t> _activeTrx;
    long _numberObjects;
  };
  std::vector<CacheDeleterUnitDiagnostic> _units;
  size_t _cacheSize;
  size_t _uncompressedSize;
  size_t _objectsInAccumulator;
  size_t _totalObjects;
  size_t _objectsInUncompressed;
};
}