//----------------------------------------------------------------------------
//  File:        MarketRoutingCacheUpdater.cpp
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

#include "FileLoader/MarketRoutingCacheUpdater.h"

#include "Allocator/TrxMalloc.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/Logger.h"
#include "FileLoader/BFUtils.h"
#include "FileLoader/MarketRoutingLoader.h"

#include <log4cxx/logger.h>
#include <iostream>

#include <ZThreads/zthread/Exceptions.h>
#include <ZThreads/zthread/ThreadedExecutor.h>

namespace tse
{
const std::string
MRFilePatterns::MARKETS("markets.load.txt.");
const std::string
MRFilePatterns::SINGLES("singles.load.txt.");
const std::string
MRFilePatterns::DOUBLES("doubles.load.txt.");
const std::string
MRFilePatterns::GZ("gz");

ZThread::ThreadedExecutor MarketRoutingCacheUpdater::_executor;
static Logger
logger("atseintl.MarketRoutingCacheUpdater");
bool
MarketRoutingCacheUpdater::_esvUpdateInProgress(false);

namespace
{
ConfigurableValue<bool>
esvLogic("SHOPPING_OPT", "ESV_LOGIC");
ConfigurableValue<bool>
cacheUpdateEnabled("BOUND_FARE", "ENABLE_CACHE_UPDATE");
}

MarketRoutingCacheUpdater::MarketRoutingCacheUpdater(const std::string& dataDir,
                                                     const std::string& marketsFile,
                                                     const std::string& singlesFile,
                                                     const std::string& doublesFile,
                                                     const uint32_t generationNumber,
                                                     MarketRoutingCache* cache)
  : _dataDir(dataDir),
    _marketsFile(marketsFile),
    _singlesFile(singlesFile),
    _doublesFile(doublesFile),
    _generationNumber(generationNumber),
    _cache(cache)
{
  LOG4CXX_DEBUG(logger, "MarketRoutingCacheUpdater::MarketRoutingCacheUpdater()");

  _esvUpdateInProgress = true;
}

MarketRoutingCacheUpdater::~MarketRoutingCacheUpdater()
{
  LOG4CXX_DEBUG(logger, "MarketRoutingCacheUpdater::~MarketRoutingCacheUpdater()");

  _esvUpdateInProgress = false;
}

void
MarketRoutingCacheUpdater::run()
{
  const MallocContextDisabler context;

  LOG4CXX_DEBUG(logger, "MarketRoutingCacheUpdater::run()");

  try
  {
    MarketRoutingLoader loader(
        _dataDir, _marketsFile, _singlesFile, _doublesFile, _generationNumber, _cache);

    loader.parse();
  }
  catch (const std::exception& e)
  {
    LOG4CXX_ERROR(logger, e.what());
  }
}

void
MarketRoutingCacheUpdater::checkForUpdates(MarketRoutingCache* cache,
                                           const std::string& dataDir,
                                           int32_t& routingLastLoadGeneration)
{
  if (!esvLogic.getValue())
    return;

  if (!cacheUpdateEnabled.getValue())
    return;

  if (nullptr == cache)
  {
    LOG4CXX_ERROR(logger, "MarketRoutingCacheUpdater::checkForUpdates() - Cache object is NULL.");
    return;
  }

  if (dataDir.empty())
  {
    LOG4CXX_ERROR(logger,
                  "MarketRoutingCacheUpdater::checkForUpdates() - Data directory is empty.");
    return;
  }

  if (true == _esvUpdateInProgress)
  {
    LOG4CXX_INFO(
        logger,
        "MarketRoutingCacheUpdater::checkForUpdates() - BF cache update is already in progress.");
    return;
  }

  checkUpdateFiles(cache, dataDir, routingLastLoadGeneration);
}

void
MarketRoutingCacheUpdater::checkUpdateFiles(MarketRoutingCache* cache,
                                            const std::string& dataDir,
                                            int32_t& routingLastLoadGeneration)
{
  const MallocContextDisabler context;

  std::string marketsFile = "";
  std::string singlesFile = "";
  std::string doublesFile = "";

  int32_t generation = -1;

  std::vector<int32_t> marketsGenerationsVec;
  std::vector<int32_t> singlesGenerationsVec;
  std::vector<int32_t> doublesGenerationsVec;

  // Markets
  BFUtils::getAllFileGenerations(
      dataDir, MRFilePatterns::MARKETS, "." + MRFilePatterns::GZ, marketsGenerationsVec);

  if (marketsGenerationsVec.empty())
  {
    return;
  }

  // Singles
  BFUtils::getAllFileGenerations(
      dataDir, MRFilePatterns::SINGLES, "." + MRFilePatterns::GZ, singlesGenerationsVec);

  if (singlesGenerationsVec.empty())
  {
    return;
  }

  // Doubles
  BFUtils::getAllFileGenerations(
      dataDir, MRFilePatterns::DOUBLES, "." + MRFilePatterns::GZ, doublesGenerationsVec);

  if (doublesGenerationsVec.empty())
  {
    return;
  }

  // Try to findh highest generation of file which exist for markets,
  // routings and singles files
  for (int32_t i = marketsGenerationsVec.size() - 1; i >= 0; --i)
  {
    int32_t highestMarketsGen = marketsGenerationsVec.at(i);

    if ((singlesGenerationsVec.end() != std::find(singlesGenerationsVec.begin(),
                                                  singlesGenerationsVec.end(),
                                                  highestMarketsGen)) &&
        (doublesGenerationsVec.end() !=
         std::find(doublesGenerationsVec.begin(), doublesGenerationsVec.end(), highestMarketsGen)))
    {
      generation = highestMarketsGen;
      break;
    }
  }

  if (-1 == generation)
  {
    return;
  }

  time_t marketsTimestamp = 0;
  time_t singlesTimestamp = 0;
  time_t doublesTimestamp = 0;

  if (generation > routingLastLoadGeneration)
  {
    marketsTimestamp = BFUtils::getLatestFile(
        dataDir, MRFilePatterns::MARKETS, MRFilePatterns::GZ, marketsFile, generation);
  }

  if (generation > routingLastLoadGeneration)
  {
    singlesTimestamp = BFUtils::getLatestFile(
        dataDir, MRFilePatterns::SINGLES, MRFilePatterns::GZ, singlesFile, generation);
  }

  if (generation > routingLastLoadGeneration)
  {
    doublesTimestamp = BFUtils::getLatestFile(
        dataDir, MRFilePatterns::DOUBLES, MRFilePatterns::GZ, doublesFile, generation);
  }

  if ((marketsTimestamp != 0) && (singlesTimestamp != 0) && (doublesTimestamp != 0))
  {
    routingLastLoadGeneration = generation;

    ZThread::Task cacheUpdater(new MarketRoutingCacheUpdater(
        dataDir, marketsFile, singlesFile, doublesFile, routingLastLoadGeneration, cache));
    try
    {
      _executor.execute(cacheUpdater);

      _executor.wait(120000); // Wait for loading routings
      // (setup 2 minutes timeout)

      LOG4CXX_INFO(logger,
                   "MarketRoutingCacheUpdater::checkUpdateFiles() - Routing markets file: \""
                       << dataDir << marketsFile << ", Generation: " << generation << " loaded.");
      LOG4CXX_INFO(logger,
                   "MarketRoutingCacheUpdater::checkUpdateFiles() - Routing singles file: \""
                       << dataDir << singlesFile << ", Generation: " << generation << " loaded.");
      LOG4CXX_INFO(logger,
                   "MarketRoutingCacheUpdater::checkUpdateFiles() - Routing doubles file: \""
                       << dataDir << doublesFile << ", Generation: " << generation << " loaded.");

      return;
    }
    catch (boost::thread_interrupted&)
    {
      LOG4CXX_ERROR(logger, "MarketRoutingCacheUpdater::checkUpdateFiles: interrupted");
    }
    catch (ZThread::Synchronization_Exception& e)
    {
      LOG4CXX_ERROR(logger, "MarketRoutingCacheUpdater::checkUpdateFiles:" << e.what());
    }
  }
}
} // namespace tse
