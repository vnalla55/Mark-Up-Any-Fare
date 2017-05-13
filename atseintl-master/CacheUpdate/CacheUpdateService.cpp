//----------------------------------------------------------------------------
//
//  File:               CacheUpdateService.C
//  Description:        Service class to process
//                      Cache Update Notification Transactions
//
//  Copyright Sabre 2004
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------

#include "CacheUpdate/CacheUpdateService.h"

#include "Common/CacheStats.h"
#include "Common/ErrorResponseException.h"
#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/MetricsUtil.h"
#include "Common/StopWatch.h"
#include "Common/TSELatencyData.h"
#include "Common/XMLConstruct.h"
#include "DataModel/CacheTrx.h"
#include "DataModel/Response.h"
#include "DataModel/Trx.h"
#include "DBAccess/CacheControl.h"
#include "DBAccess/CacheManager.h"
#include "DBAccess/CacheRegistry.h"
#include "DBAccess/DAOUtils.h"
#include "DBAccess/ObjectKey.h"

#include <iomanip>

namespace tse
{
static Logger
logger("atseintl.CacheUpdate.CacheUpdateService");

bool
CacheUpdateService::process(CacheTrx& trx)
{
  TSELatencyData latency(trx, "CacheUpdateService");

  CacheRegistry& registry = CacheRegistry::instance();
  double cpuTime;
  uint64_t cacheSize;
  int64_t deleteCount = 0;
  const CacheUpdatePtrVector& events(trx.cacheEvents());
  CacheUpdatePtrVector::const_iterator i = events.begin();

  for (; i != events.end(); ++i)
  {
    MetricsTimer timer;
    timer.start();

    CacheControl* ctrl = registry.getCacheControl((*i)->_key.tableName());
    if (ctrl == nullptr)
    {
      LOG4CXX_INFO(logger,
                   "Cache update message not processed: table " << (*i)->_key.tableName()
                                                                << " isn't in cache");
    }
    else
    {
      CacheStats* stats = ctrl->cacheStats();
      CacheStats safetyStats;
      if (stats == nullptr)
        stats = &safetyStats;

      bool ignored(false);
      (*i)->_ctrl = ctrl;
      switch ((*i)->_action)
      {
      case CACHE_UPDATE_INVALIDATE:
        stats->updates++;
        deleteCount = 0;
        cacheSize = ctrl->cacheSize();
        if (ctrl->nodupInvalidate((*i)->_key))
        {
          ignored = false;
          deleteCount = cacheSize - ctrl->cacheSize();
        }
        else
        {
          ignored = true;
        }
        if (deleteCount == 0)
          stats->noneDeleted++;
        if (deleteCount > 0)
          stats->deletes += deleteCount;
        break;

      case CACHE_UPDATE_CLEAR:
        stats->flushes++;
        cacheSize = ctrl->cacheSize();
        ctrl->clear();
        deleteCount = cacheSize - ctrl->cacheSize();
        if (deleteCount >= 0)
        {
          stats->deletes += deleteCount;
          if (deleteCount == 0)
            stats->noneDeleted++;
        }
        break;

      default:
        break;
      }
      timer.stop();
      cpuTime = timer.getCpuUserTime() + timer.getCpuSystemTime();
      stats->cpuTime += cpuTime;
      if (logger->isInfoEnabled())
      {
        std::ostringstream os;
        const ObjectKey& key((*i)->_key);
        ObjectKey::KeyFields::const_iterator j = key.keyFields().begin();
        for (; j != key.keyFields().end(); ++j)
          os << " " << j->first << "=" << j->second;
        if ((*i)->_action == CACHE_UPDATE_CLEAR)
          os << " Flush";
        if (deleteCount > 0)
          os << " deleted " << deleteCount << " entries";
        if (ignored)
          os << " ignored";
        LOG4CXX_INFO(logger, key.tableName() << ": " << os.str());
      }
    }
  }
  return true;
}

static LoadableModuleRegister<Service, CacheUpdateService>
_("libCacheUpdate.so");
} // tse namespace
