//----------------------------------------------------------------------------
//  File:        BFCacheUpdate.cpp
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

#include "FileLoader/BFCacheUpdate.h"

#include "Allocator/TrxMalloc.h"
#include "Common/Config/ConfigMan.h"
#include "Common/Config/ConfigManUtils.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/TseStringTypes.h"
#include "Common/TseUtil.h"
#include "DBAccess/Cache.h"
#include "DBAccess/HashKey.h"
#include "FileLoader/BFUtils.h"
#include "FileLoader/FileLoader.h"

#include <iostream>

#include <ZThreads/zthread/Exceptions.h>

namespace tse
{

const std::string
BFFilePatterns::LOAD("all-domestic-fares.sorted.load.");
const std::string
BFFilePatterns::GZ("gz");

ZThread::ThreadedExecutor BFCacheUpdate::_executor;
static Logger
logger("atseintl.BFCacheUpdate");
bool
BFCacheUpdate::_esvUpdateInProgress(false);

namespace
{
ConfigurableValue<bool>
esvLogic("SHOPPING_OPT", "ESV_LOGIC");
ConfigurableValue<bool>
cacheUpdateEnabled("BOUND_FARE", "ENABLE_CACHE_UPDATE");
}

BFCacheUpdate::BFCacheUpdate(const std::string& url, BoundFareCache* cache)
  : _url(url), _cache(cache)
{
  LOG4CXX_DEBUG(logger, "BFCacheUpdate::BFCacheUpdate()");

  _esvUpdateInProgress = true;
}

BFCacheUpdate::~BFCacheUpdate()
{
  LOG4CXX_DEBUG(logger, "BFCacheUpdate::~BFCacheUpdate()");

  _esvUpdateInProgress = false;
}

void
BFCacheUpdate::run()
{
  const MallocContextDisabler context;

  LOG4CXX_DEBUG(logger, "BFCacheUpdate::run()");

  FileLoader loader(_url, _cache);

  try { loader.parse(); }
  catch (const std::exception& e) { LOG4CXX_ERROR(logger, e.what()); }
}

void
BFCacheUpdate::checkForUpdates(BoundFareCache* cache,
                               const std::string& dataDir,
                               int32_t& lastGeneration)
{
  if (!esvLogic.getValue())
    return;
  if (!cacheUpdateEnabled.getValue())
    return;

  if (nullptr == cache)
  {
    LOG4CXX_ERROR(logger, "BFCacheUpdate::checkForUpdates() - Cache object is NULL.");
    return;
  }

  if (dataDir.empty())
  {
    LOG4CXX_ERROR(logger, "BFCacheUpdate::checkForUpdates() - Data directory is empty.");
    return;
  }

  if (true == _esvUpdateInProgress)
  {
    LOG4CXX_INFO(logger,
                 "BFCacheUpdate::checkForUpdates() - BF cache update is already in progress.");
    return;
  }

  checkFiles(cache, dataDir, lastGeneration, BFFilePatterns::LOAD, BFFilePatterns::GZ);

}

void
BFCacheUpdate::checkFiles(BoundFareCache* cache,
                          const std::string& dataDir,
                          int32_t& lastGeneration,
                          const std::string& prefix,
                          const std::string& suffix)
{
  const MallocContextDisabler context;

  int32_t requestedGeneration = -1;
  std::vector<int32_t> generationsVec;

  BFUtils::getAllFileGenerations(
      dataDir, BFFilePatterns::LOAD, "." + BFFilePatterns::GZ, generationsVec);
  if (generationsVec.empty())
  {
    return;
  }
  else
  {
    // Pick highest found generation
    requestedGeneration = generationsVec.at(generationsVec.size() - 1);
  }

  if (-1 == requestedGeneration)
  {
    processFile(cache, dataDir, prefix, suffix, lastGeneration);
  }
  else
  {
    // For full load use only latest availabale file
    if (requestedGeneration > lastGeneration)
    {
      processFile(cache, dataDir, prefix, suffix, lastGeneration, requestedGeneration);
    }
  }
}

void
BFCacheUpdate::processFile(BoundFareCache* cache,
                           const std::string& dataDir,
                           const std::string& prefix,
                           const std::string& suffix,
                           int32_t& lastGeneration,
                           int32_t requestedGeneration)
{
  std::string latestFile = "";

  time_t timestamp =
      BFUtils::getLatestFile(dataDir, prefix, suffix, latestFile, requestedGeneration);

  if (timestamp != 0)
  {
    lastGeneration = requestedGeneration;

    try
    {
      _executor.execute(ZThread::Task(new BFCacheUpdate(dataDir + latestFile, cache)));

      _executor.wait(240000); // Wait for loading fares
      // (setup 4 minutes timeout)

      LOG4CXX_INFO(logger,
                   "BFCacheUpdate::processFile() - Bound fare file: \""
                       << dataDir << latestFile << "\", Timestamp: " << timestamp
                       << ", Generation: " << requestedGeneration << " loaded.");

      return;
    }
    catch (boost::thread_interrupted&)
    {
      LOG4CXX_ERROR(logger, "BFCacheUpdate::processFile() - interrupted");
    }
    catch (ZThread::Synchronization_Exception& e)
    {
      LOG4CXX_ERROR(logger, "BFCacheUpdate::processFile() - " << e.what());
    }
  }
}

} // namespace tse
