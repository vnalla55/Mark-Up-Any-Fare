//-------------------------------------------------------------------------------
// Copyright 2006, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DBAccess/CacheInitializer.h"

#include "Common/Thread/TseScopedExecutor.h"
#include "DBAccess/DiskCache.h"

namespace tse
{
CacheInitializer::CacheInitializer(TseScopedExecutor* scopedExecutor,
                                   std::map<std::string, CacheManager::CacheParm>& cacheParms)
  : _scopedExecutor(scopedExecutor),
    _cacheParms(cacheParms),
    _logger("atseintl.DBAccess.CacheInitializer")
{
}

CacheInitializer::~CacheInitializer()
{
  if (_scopedExecutor)
  {
    _scopedExecutor->wait();
  }
}

void
CacheInitializer::
operator()(std::pair<const std::string, CacheControl*>& p)
{
  const std::string& cacheName(p.first);
  CacheControl& ctl = *(p.second);
  CacheManager::CacheParm& parm = _cacheParms[p.first];

  DiskCache::CacheTypeOptions* cto = DISKCACHE.getCacheTypeOptions(cacheName);
  bool ldcEnabled = ((cto != nullptr) && (cto->enabled));

  if (parm.loadOnStart || ldcEnabled)
  {
    if (parm.loadOnStart)
    {
      LOG4CXX_TRACE(_logger, "Initializing " << cacheName << " cache");
    }
    else
    {
      LOG4CXX_TRACE(_logger, "Initializing " << cacheName << " cache (for LDC only)");
    }
    _tasks.push_back(CacheInitializationTask(&ctl));
    if (_scopedExecutor)
    {
      _scopedExecutor->execute(_tasks.back());
    }
  }
  else
  {
    LOG4CXX_TRACE(_logger, "Load disabled for " << cacheName << " cache");
    ctl.init(false);
  }
}
}
