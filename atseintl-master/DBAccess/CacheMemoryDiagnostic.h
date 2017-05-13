#pragma once

#include "Util/FlatSet.h"

#include <set>
#include <vector>

namespace tse
{
//TODO it's completely unused, remove?
struct CacheMemoryDiagnostic
{
  CacheMemoryDiagnostic()
    : _cacheSize(0),
      _uncompressedSize(0),
      _objectsInAccumulator(0),
      _totalObjects(0),
      _objectsInUncompressed(0)
  {
  }
  FlatSet<int64_t> _activeTrx;
  struct CacheDeleterUnitDiagnostic
  {
    CacheDeleterUnitDiagnostic() : _numberObjects(0) {}
    FlatSet<int64_t> _activeTrx;
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
