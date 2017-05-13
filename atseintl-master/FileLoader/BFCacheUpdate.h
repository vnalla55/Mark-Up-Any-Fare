//----------------------------------------------------------------------------
//  File:        BFCacheUpdate.h
//  Created:     2009-01-01
//
//  Description: Bound Fare Cache Update class
//
//  Updates:
//
//  Copyright Sabre 2009
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

#include "DBAccess/BoundFareDAO.h"

#include "ZThreads/zthread/Runnable.h"
#include "ZThreads/zthread/ThreadedExecutor.h"

namespace tse
{

struct BFFilePatterns
{
  static const std::string LOAD;
  static const std::string GZ;
};

class BFCacheUpdate : public ZThread::Runnable
{
public:
  BFCacheUpdate(const std::string& url, BoundFareCache* cache);

  virtual ~BFCacheUpdate();

  void run() override;

  static void
  checkForUpdates(BoundFareCache* cache, const std::string& dataDir, int32_t& lastGeneration);

private:
  static void checkFiles(BoundFareCache* cache,
                         const std::string& dataDir,
                         int32_t& lastGeneration,
                         const std::string& prefix,
                         const std::string& suffix);

  static void processFile(BoundFareCache* cache,
                          const std::string& dataDir,
                          const std::string& prefix,
                          const std::string& suffix,
                          int32_t& lastGeneration,
                          int32_t requestedGeneration = -1);

  const std::string _url;
  BoundFareCache* const _cache;

  static ZThread::ThreadedExecutor _executor;
  static bool _esvUpdateInProgress;
};

} // End namespace tse

