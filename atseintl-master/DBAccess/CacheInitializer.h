//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#pragma once
#include "Common/Logger.h"
#include "DBAccess/CacheControl.h"
#include "DBAccess/CacheManager.h"
#include "DBAccess/DataAccessObject.h"

#include <list>

namespace tse
{

class TseScopedExecutor;

class CacheInitializer
{
public:
  class CacheInitializationTask : public TseCallableTask
  {
  public:
    CacheInitializationTask(CacheControl* ctl) : _ctl(ctl) {}

    void run() override { _ctl->init(true); }

  private:
    CacheControl* _ctl;
  };

  CacheInitializer(TseScopedExecutor* scopedExecutor,
                   std::map<std::string, CacheManager::CacheParm>& cacheParms);
  ~CacheInitializer();

  void operator()(std::pair<const std::string, CacheControl*>& p);

private:
  TseScopedExecutor* _scopedExecutor;
  std::map<std::string, CacheManager::CacheParm>& _cacheParms;
  Logger _logger;
  std::list<CacheInitializationTask> _tasks;
};

} // namespace tse
