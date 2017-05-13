//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#pragma once

namespace tse
{
struct CacheStats
{
  CacheStats()
    : updates(0),
      flushes(0),
      deletes(0),
      noneDeleted(0),
      cpuTime(0),
      rowsAdded(0),
      rowsRemoved(0) {};

  uint64_t updates;
  uint64_t flushes;
  uint64_t deletes;
  uint64_t noneDeleted;
  double cpuTime;
  uint64_t rowsAdded;
  uint64_t rowsRemoved;
};
}

