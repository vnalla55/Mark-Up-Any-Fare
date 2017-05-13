//----------------------------------------------------------------------------
//  File:        MarketRoutingCacheUpdater.h
//  Created:     2009-01-01
//
//  Description: Routing cache updater
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

#include "FileLoader/MarketRoutingMap.h"

#include "ZThreads/zthread/Runnable.h"
#include "ZThreads/zthread/ThreadedExecutor.h"

namespace tse
{

struct MRFilePatterns
{
  static const std::string MARKETS;
  static const std::string SINGLES;
  static const std::string DOUBLES;
  static const std::string GZ;
};

class MarketRoutingCacheUpdater : public ZThread::Runnable
{
public:
  MarketRoutingCacheUpdater(const std::string& dataDir,
                            const std::string& marketsFile,
                            const std::string& singlesFile,
                            const std::string& doublesFile,
                            const uint32_t generationNumber,
                            MarketRoutingCache* cache);

  virtual ~MarketRoutingCacheUpdater();

  void run() override;

  static void checkForUpdates(MarketRoutingCache* cache,
                              const std::string& dataDir,
                              int32_t& routingLastLoadGeneration);

private:
  static void checkUpdateFiles(MarketRoutingCache* cache,
                               const std::string& dataDir,
                               int32_t& routingLastLoadGeneration);

  std::string _dataDir;
  std::string _marketsFile;
  std::string _singlesFile;
  std::string _doublesFile;
  uint32_t _generationNumber;
  MarketRoutingCache* const _cache;

  static ZThread::ThreadedExecutor _executor;
  static bool _esvUpdateInProgress;
};

} // End namespace tse

