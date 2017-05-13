//----------------------------------------------------------------------------
//  File:           QueryGetCacheNotify.cpp
//  Description:    QueryGetCacheNotify
//  Created:        5/24/2006
// Authors:         Mike Lillis
//
//  Updates:
//
// 2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetCacheNotify.h"
#include "Adapter/CacheNotifyAdapter.h"
#include "Common/FallbackUtil.h"
#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCacheNotifySQLStatement.h"

#include <boost/algorithm/string.hpp>

namespace tse
{

FIXEDFALLBACK_DECL(fallbackFilterCacheNotify);

namespace
{

uint32_t
initializeProcessingDelay()
{
  log4cxx::LoggerPtr cacheNotifyLogger(
      log4cxx::Logger::getLogger("atseintl.Adapter.CacheNotifyAdapter"));
  const MallocContextDisabler context;
  tse::ConfigMan& config = Global::config();
  std::string name("CACHE_ADP");
  uint32_t delay = 0;

  // PROCESSING_DELAY support random range: 600..1800
  std::string s;
  if (config.getValue("PROCESSING_DELAY", s, name))
  {
    size_t nDots = s.find("..");
    if (nDots == std::string::npos)
    {
      delay = boost::lexical_cast<unsigned int>(s);
    }
    else
    {
      std::string sLower(s.substr(0, nDots));
      std::string sUpper(s.substr(nDots + 2));
      unsigned int nLower(boost::lexical_cast<unsigned int>(sLower));
      unsigned int nUpper(boost::lexical_cast<unsigned int>(sUpper));
      if (nLower > nUpper)
        std::swap(nLower, nUpper);
      srand(time(nullptr)); // initialize random number generator
      delay = nLower + (rand() % (nUpper - nLower + 1));
    }
  }
  else
  {
    LOG4CXX_INFO(cacheNotifyLogger, "Couldn't find config entry for 'PROCESSING_DELAY'");
    delay = 0;
  }
  return delay;
}

void filterEntityTypes(std::vector<CacheNotifyInfo>& infos)
{
  if (!fallback::fixed::fallbackFilterCacheNotify())
  {
    const CacheNotifyControlInitializer& cacheControl(CacheNotifyControlInitializer::instance());
    if (cacheControl.isValid())
    {
      std::vector<CacheNotifyInfo> filtered;
      for (auto& info : infos)
      {
        const std::string& entity(info.entityType());
        if (cacheControl.inUse(entity))
        {
          filtered.push_back(info);
        }
        else
        {
          //std::cerr << "!!!!! entity:" << entity << std::endl;
        }
      }
      infos.swap(filtered);
    }
  }
}

}// namespace

uint32_t
QueryCacheNotificationGlobals::processingDelay()
{
  static uint32_t delay = initializeProcessingDelay();
  return delay;
}

///////////////////////////////////////////////////////////
//
//  QueryGetAllFareCacheNotify
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllFareCacheNotify::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllFareCacheNotify"));
std::string QueryGetAllFareCacheNotify::_baseSQL;
bool QueryGetAllFareCacheNotify::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllFareCacheNotify> g_GetAllFareCacheNotify;

const char*
QueryGetAllFareCacheNotify::getQueryName() const
{
  return "GETALLFARECACHENOTIFY";
}

void
QueryGetAllFareCacheNotify::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllFareCacheNotifySQLStatement<QueryGetAllFareCacheNotify> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLFARECACHENOTIFY");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllFareCacheNotify::findAllFareCacheNotify(std::vector<tse::CacheNotifyInfo>& infos,
                                                   int lastId,
                                                   int pollSize)
{
  substParm(1, lastId);
  substParm(2, pollSize);

  DBResultSet res(_dbAdapt);
  res.executeQuery(this);
  QueryGetAllFareCacheNotifySQLStatement<QueryGetAllFareCacheNotify>::mapResultToCacheNotify(infos,
                                                                                             res);
  res.freeResult();
  filterEntityTypes(infos);
} // findAllFareCacheNotify()

///////////////////////////////////////////////////////////
//
//  QueryGetAllRuleCacheNotify
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllRuleCacheNotify::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllRuleCacheNotify"));
std::string QueryGetAllRuleCacheNotify::_baseSQL;
bool QueryGetAllRuleCacheNotify::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllRuleCacheNotify> g_GetAllRuleCacheNotify;

const char*
QueryGetAllRuleCacheNotify::getQueryName() const
{
  return "GETALLRULECACHENOTIFY";
}

void
QueryGetAllRuleCacheNotify::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllRuleCacheNotifySQLStatement<QueryGetAllRuleCacheNotify> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLRULECACHENOTIFY");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllRuleCacheNotify::findAllRuleCacheNotify(std::vector<tse::CacheNotifyInfo>& infos,
                                                   int lastId,
                                                   int pollSize)
{
  substParm(1, lastId);
  substParm(2, pollSize);

  DBResultSet res(_dbAdapt);
  res.executeQuery(this);
  QueryGetAllRuleCacheNotifySQLStatement<QueryGetAllRuleCacheNotify>::mapResultToCacheNotify(infos,
                                                                                             res);
  res.freeResult();
  filterEntityTypes(infos);
} // findAllRuleCacheNotify()

///////////////////////////////////////////////////////////
//
//  QueryGetAllRoutingCacheNotify
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllRoutingCacheNotify::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllRoutingCacheNotify"));
std::string QueryGetAllRoutingCacheNotify::_baseSQL;
bool QueryGetAllRoutingCacheNotify::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllRoutingCacheNotify> g_GetAllRoutingCacheNotify;

const char*
QueryGetAllRoutingCacheNotify::getQueryName() const
{
  return "GETALLROUTINGCACHENOTIFY";
}

void
QueryGetAllRoutingCacheNotify::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllRoutingCacheNotifySQLStatement<QueryGetAllRoutingCacheNotify> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLROUTINGCACHENOTIFY");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllRoutingCacheNotify::findAllRoutingCacheNotify(std::vector<tse::CacheNotifyInfo>& infos,
                                                         int lastId,
                                                         int pollSize)
{
  substParm(1, lastId);
  substParm(2, pollSize);

  DBResultSet res(_dbAdapt);
  res.executeQuery(this);
  QueryGetAllRoutingCacheNotifySQLStatement<QueryGetAllRoutingCacheNotify>::mapResultToCacheNotify(
      infos, res);
  res.freeResult();
  filterEntityTypes(infos);
} // findAllRoutingCacheNotify()

///////////////////////////////////////////////////////////
//
//  QueryGetAllSupportCacheNotify
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllSupportCacheNotify::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllSupportCacheNotify"));
std::string QueryGetAllSupportCacheNotify::_baseSQL;
bool QueryGetAllSupportCacheNotify::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllSupportCacheNotify> g_GetAllSupportCacheNotify;

const char*
QueryGetAllSupportCacheNotify::getQueryName() const
{
  return "GETALLSUPPORTCACHENOTIFY";
}

void
QueryGetAllSupportCacheNotify::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllSupportCacheNotifySQLStatement<QueryGetAllSupportCacheNotify> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLSUPPORTCACHENOTIFY");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllSupportCacheNotify::findAllSupportCacheNotify(std::vector<tse::CacheNotifyInfo>& infos,
                                                         int lastId,
                                                         int pollSize)
{
  substParm(1, lastId);
  substParm(2, pollSize);

  DBResultSet res(_dbAdapt);
  res.executeQuery(this);
  QueryGetAllSupportCacheNotifySQLStatement<QueryGetAllSupportCacheNotify>::mapResultToCacheNotify(
      infos, res);
  res.freeResult();
  filterEntityTypes(infos);
} // findAllSupportCacheNotify()

///////////////////////////////////////////////////////////
//
//  QueryGetAllIntlCacheNotify
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllIntlCacheNotify::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllIntlCacheNotify"));
std::string QueryGetAllIntlCacheNotify::_baseSQL;
bool QueryGetAllIntlCacheNotify::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllIntlCacheNotify> g_GetAllIntlCacheNotify;

const char*
QueryGetAllIntlCacheNotify::getQueryName() const
{
  return "GETALLINTLCACHENOTIFY";
}

void
QueryGetAllIntlCacheNotify::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllIntlCacheNotifySQLStatement<QueryGetAllIntlCacheNotify> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLINTLCACHENOTIFY");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllIntlCacheNotify::findAllIntlCacheNotify(std::vector<tse::CacheNotifyInfo>& infos,
                                                   int lastId,
                                                   int pollSize)
{
  substParm(1, lastId);
  substParm(2, pollSize);

  DBResultSet res(_dbAdapt);
  res.executeQuery(this);
  QueryGetAllIntlCacheNotifySQLStatement<QueryGetAllIntlCacheNotify>::mapResultToCacheNotify(infos,
                                                                                             res);
  res.freeResult();
  filterEntityTypes(infos);
} // findAllIntlCacheNotify()

///////////////////////////////////////////////////////////
//
//  QueryGetAllMerchandisingCacheNotify
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllMerchandisingCacheNotify::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllMerchandisingCacheNotify"));
std::string QueryGetAllMerchandisingCacheNotify::_baseSQL;
bool QueryGetAllMerchandisingCacheNotify::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllMerchandisingCacheNotify> g_GetAllMerchandisingCacheNotify;

const char*
QueryGetAllMerchandisingCacheNotify::getQueryName() const
{
  return "GETALLMERCHANDISINGCACHENOTIFY";
}

void
QueryGetAllMerchandisingCacheNotify::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllMerchandisingCacheNotifySQLStatement<QueryGetAllMerchandisingCacheNotify>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLMERCHANDISINGCACHENOTIFY");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllMerchandisingCacheNotify::findAllMerchandisingCacheNotify(
    std::vector<tse::CacheNotifyInfo>& infos, int lastId, int pollSize)
{
  substParm(1, lastId);
  substParm(2, pollSize);

  DBResultSet res(_dbAdapt);
  res.executeQuery(this);
  QueryGetAllMerchandisingCacheNotifySQLStatement<
      QueryGetAllMerchandisingCacheNotify>::mapResultToCacheNotify(infos, res);
  res.freeResult();
  filterEntityTypes(infos);
} // findAllMerchandisingCacheNotify()

///////////////////////////////////////////////////////////
//
//  QueryGetAllCacheNotifyHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllCacheNotifyHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllCacheNotifyHistorical"));
std::string QueryGetAllCacheNotifyHistorical::_baseSQL;
bool QueryGetAllCacheNotifyHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllCacheNotifyHistorical> g_GetAllCacheNotifyHistorical;

const char*
QueryGetAllCacheNotifyHistorical::getQueryName() const
{
  return "GETALLCACHENOTIFYHISTORICAL";
}

void
QueryGetAllCacheNotifyHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllCacheNotifyHistoricalSQLStatement<QueryGetAllCacheNotifyHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLCACHENOTIFYHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllCacheNotifyHistorical::findAllCacheNotifyHistorical(
    std::vector<tse::CacheNotifyInfo>& infos, int lastId, int pollSize)
{
  substParm(1, lastId);
  substParm(2, pollSize);

  DBResultSet res(_dbAdapt);
  res.executeQuery(this);
  QueryGetAllCacheNotifyHistoricalSQLStatement<
      QueryGetAllCacheNotifyHistorical>::mapResultToCacheNotify(infos, res);
  res.freeResult();
  filterEntityTypes(infos);
} // findAllCacheNotifyHistorical()

///////////////////////////////////////////////////////////
//
//  QueryGetLastFareCacheNotify
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetLastFareCacheNotify::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetLastFareCacheNotify"));
std::string QueryGetLastFareCacheNotify::_baseSQL;
bool QueryGetLastFareCacheNotify::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetLastFareCacheNotify> g_GetLastFareCacheNotify;

const char*
QueryGetLastFareCacheNotify::getQueryName() const
{
  return "GETLASTFARECACHENOTIFY";
}

void
QueryGetLastFareCacheNotify::initialize()
{
  if (!_isInitialized)
  {
    QueryGetLastFareCacheNotifySQLStatement<QueryGetLastFareCacheNotify> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETLASTFARECACHENOTIFY");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

uint64_t
QueryGetLastFareCacheNotify::findLastFareCacheNotify(const DateTime& startTime)
{
  substParm(1, startTime);

  DBResultSet res(_dbAdapt);
  res.executeQuery(this);
  uint64_t ret =
      QueryGetLastFareCacheNotifySQLStatement<QueryGetLastFareCacheNotify>::getLastNotifyId(res);
  res.freeResult();
  return ret;
} // findLastFareCacheNotify()

///////////////////////////////////////////////////////////
//
//  QueryGetLastRuleCacheNotify
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetLastRuleCacheNotify::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetLastRuleCacheNotify"));
std::string QueryGetLastRuleCacheNotify::_baseSQL;
bool QueryGetLastRuleCacheNotify::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetLastRuleCacheNotify> g_GetLastRuleCacheNotify;

const char*
QueryGetLastRuleCacheNotify::getQueryName() const
{
  return "GETLASTRULECACHENOTIFY";
}

void
QueryGetLastRuleCacheNotify::initialize()
{
  if (!_isInitialized)
  {
    QueryGetLastRuleCacheNotifySQLStatement<QueryGetLastRuleCacheNotify> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETLASTRULECACHENOTIFY");

    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

uint64_t
QueryGetLastRuleCacheNotify::findLastRuleCacheNotify(const DateTime& startTime)
{
  substParm(1, startTime);

  DBResultSet res(_dbAdapt);
  res.executeQuery(this);
  uint64_t ret =
      QueryGetLastRuleCacheNotifySQLStatement<QueryGetLastRuleCacheNotify>::getLastNotifyId(res);
  res.freeResult();
  return ret;
} // findLastRuleCacheNotify()

///////////////////////////////////////////////////////////
//
//  QueryGetLastRoutingCacheNotify
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetLastRoutingCacheNotify::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetLastRoutingCacheNotify"));
std::string QueryGetLastRoutingCacheNotify::_baseSQL;
bool QueryGetLastRoutingCacheNotify::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetLastRoutingCacheNotify> g_GetLastRoutingCacheNotify;

const char*
QueryGetLastRoutingCacheNotify::getQueryName() const
{
  return "GETLASTROUTINGCACHENOTIFY";
}

void
QueryGetLastRoutingCacheNotify::initialize()
{
  if (!_isInitialized)
  {
    QueryGetLastRoutingCacheNotifySQLStatement<QueryGetLastRoutingCacheNotify> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETLASTROUTINGCACHENOTIFY");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

uint64_t
QueryGetLastRoutingCacheNotify::findLastRoutingCacheNotify(const DateTime& startTime)
{
  substParm(1, startTime);

  DBResultSet res(_dbAdapt);
  res.executeQuery(this);
  uint64_t ret =
      QueryGetLastRoutingCacheNotifySQLStatement<QueryGetLastRoutingCacheNotify>::getLastNotifyId(
          res);
  res.freeResult();
  return ret;
} // findLastRoutingCacheNotify()

///////////////////////////////////////////////////////////
//
//  QueryGetLastSupportCacheNotify
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetLastSupportCacheNotify::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetLastSupportCacheNotify"));
std::string QueryGetLastSupportCacheNotify::_baseSQL;
bool QueryGetLastSupportCacheNotify::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetLastSupportCacheNotify> g_GetLastSupportCacheNotify;

const char*
QueryGetLastSupportCacheNotify::getQueryName() const
{
  return "GETLASTSUPPORTCACHENOTIFY";
}

void
QueryGetLastSupportCacheNotify::initialize()
{
  if (!_isInitialized)
  {
    QueryGetLastSupportCacheNotifySQLStatement<QueryGetLastSupportCacheNotify> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETLASTSUPPORTCACHENOTIFY");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

uint64_t
QueryGetLastSupportCacheNotify::findLastSupportCacheNotify(const DateTime& startTime)
{
  substParm(1, startTime);

  DBResultSet res(_dbAdapt);
  res.executeQuery(this);
  uint64_t ret =
      QueryGetLastSupportCacheNotifySQLStatement<QueryGetLastSupportCacheNotify>::getLastNotifyId(
          res);
  res.freeResult();
  return ret;
} // findLastSupportCacheNotify()

///////////////////////////////////////////////////////////
//
//  QueryGetLastIntlCacheNotify
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetLastIntlCacheNotify::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetLastIntlCacheNotify"));
std::string QueryGetLastIntlCacheNotify::_baseSQL;
bool QueryGetLastIntlCacheNotify::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetLastIntlCacheNotify> g_GetLastIntlCacheNotify;

const char*
QueryGetLastIntlCacheNotify::getQueryName() const
{
  return "GETLASTINTLCACHENOTIFY";
}

void
QueryGetLastIntlCacheNotify::initialize()
{
  if (!_isInitialized)
  {
    QueryGetLastIntlCacheNotifySQLStatement<QueryGetLastIntlCacheNotify> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETLASTINTLCACHENOTIFY");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

uint64_t
QueryGetLastIntlCacheNotify::findLastIntlCacheNotify(const DateTime& startTime)
{
  substParm(1, startTime);

  DBResultSet res(_dbAdapt);
  res.executeQuery(this);
  uint64_t ret =
      QueryGetLastIntlCacheNotifySQLStatement<QueryGetLastIntlCacheNotify>::getLastNotifyId(res);
  res.freeResult();
  return ret;
} // findLastIntlCacheNotify()

///////////////////////////////////////////////////////////
//
//  QueryGetLastMerchandisingCacheNotify
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetLastMerchandisingCacheNotify::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetLastMerchandisingCacheNotify"));
std::string QueryGetLastMerchandisingCacheNotify::_baseSQL;
bool QueryGetLastMerchandisingCacheNotify::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetLastMerchandisingCacheNotify> g_GetLastMerchandisingCacheNotify;

const char*
QueryGetLastMerchandisingCacheNotify::getQueryName() const
{
  return "GETLASTMERCHANDISINGCACHENOTIFY";
}

void
QueryGetLastMerchandisingCacheNotify::initialize()
{
  if (!_isInitialized)
  {
    QueryGetLastMerchandisingCacheNotifySQLStatement<QueryGetLastMerchandisingCacheNotify>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETLASTMERCHANDISINGCACHENOTIFY");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

uint64_t
QueryGetLastMerchandisingCacheNotify::findLastMerchandisingCacheNotify(const DateTime& startTime)
{
  substParm(1, startTime);

  DBResultSet res(_dbAdapt);
  res.executeQuery(this);
  uint64_t ret = QueryGetLastMerchandisingCacheNotifySQLStatement<
      QueryGetLastMerchandisingCacheNotify>::getLastNotifyId(res);
  res.freeResult();
  return ret;
} // findLastMerchandisingCacheNotify()

///////////////////////////////////////////////////////////
//
//  QueryGetLastCacheNotifyHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetLastCacheNotifyHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetLastCacheNotifyHistorical"));
std::string QueryGetLastCacheNotifyHistorical::_baseSQL;
bool QueryGetLastCacheNotifyHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetLastCacheNotifyHistorical> g_GetLastCacheNotifyHistorical;

const char*
QueryGetLastCacheNotifyHistorical::getQueryName() const
{
  return "GETLASTCACHENOTIFYHISTORICAL";
}

void
QueryGetLastCacheNotifyHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetLastCacheNotifyHistoricalSQLStatement<QueryGetLastCacheNotifyHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETLASTCACHENOTIFYHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

uint64_t
QueryGetLastCacheNotifyHistorical::findLastCacheNotifyHistorical(const DateTime& startTime)
{
  substParm(1, startTime);

  DBResultSet res(_dbAdapt);
  res.executeQuery(this);
  uint64_t ret = QueryGetLastCacheNotifyHistoricalSQLStatement<
      QueryGetLastCacheNotifyHistorical>::getLastNotifyId(res);
  res.freeResult();
  return ret;
} // findLastCacheNotifyHistorical()

///////////////////////////////////////////////////////////
//
//  QueryGetAllFareCacheNotifyOrderNo
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllFareCacheNotifyOrderNo::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllFareCacheNotifyOrderNo"));
std::string QueryGetAllFareCacheNotifyOrderNo::_baseSQL;
bool QueryGetAllFareCacheNotifyOrderNo::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllFareCacheNotifyOrderNo> g_GetAllFareCacheNotifyOrderNo;

const char*
QueryGetAllFareCacheNotifyOrderNo::getQueryName() const
{
  return "GETALLFARECACHENOTIFYORDERNO";
}

void
QueryGetAllFareCacheNotifyOrderNo::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllFareCacheNotifyOrderNoSQLStatement<QueryGetAllFareCacheNotifyOrderNo> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLFARECACHENOTIFYORDERNO");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllFareCacheNotifyOrderNo::findAllFareCacheNotifyOrderNo(
    std::vector<tse::CacheNotifyInfo>& infos,
    const orderNoList& lastOrderNos,
    DateTime& priorCutoff,
    const DateTime& cutoff,
    int pollSize)
{
  QueryGetAllFareCacheNotifyOrderNoSQLStatement<
      QueryGetAllFareCacheNotifyOrderNo>::parameterSubstitution(*this,
                                                                lastOrderNos,
                                                                priorCutoff,
                                                                cutoff,
                                                                pollSize);

  DBResultSet res(_dbAdapt);
  res.executeQuery(this);
  QueryGetAllFareCacheNotifyOrderNoSQLStatement<QueryGetAllFareCacheNotifyOrderNo>::mapResult(infos,
                                                                                              res);
  res.freeResult();
  filterEntityTypes(infos);
} // findAllFareCacheNotifyOrderNo()

///////////////////////////////////////////////////////////
//
//  QueryGetAllRuleCacheNotifyOrderNo
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllRuleCacheNotifyOrderNo::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllRuleCacheNotifyOrderNo"));
std::string QueryGetAllRuleCacheNotifyOrderNo::_baseSQL;
bool QueryGetAllRuleCacheNotifyOrderNo::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllRuleCacheNotifyOrderNo> g_GetAllRuleCacheNotifyOrderNo;

const char*
QueryGetAllRuleCacheNotifyOrderNo::getQueryName() const
{
  return "GETALLRULECACHENOTIFYORDERNO";
}

void
QueryGetAllRuleCacheNotifyOrderNo::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllRuleCacheNotifyOrderNoSQLStatement<QueryGetAllRuleCacheNotifyOrderNo> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLRULECACHENOTIFYORDERNO");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllRuleCacheNotifyOrderNo::findAllRuleCacheNotifyOrderNo(
    std::vector<tse::CacheNotifyInfo>& infos,
    const orderNoList& lastOrderNos,
    DateTime& priorCutoff,
    const DateTime& cutoff,
    int pollSize)
{
  QueryGetAllRuleCacheNotifyOrderNoSQLStatement<
      QueryGetAllRuleCacheNotifyOrderNo>::parameterSubstitution(*this,
                                                                lastOrderNos,
                                                                priorCutoff,
                                                                cutoff,
                                                                pollSize);
  DBResultSet res(_dbAdapt);
  res.executeQuery(this);
  QueryGetAllRuleCacheNotifyOrderNoSQLStatement<QueryGetAllRuleCacheNotifyOrderNo>::mapResult(infos,
                                                                                              res);
  res.freeResult();
  filterEntityTypes(infos);
} // findAllRuleCacheNotifyOrderNo()

///////////////////////////////////////////////////////////
//
//  QueryGetAllRoutingCacheNotifyOrderNo
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllRoutingCacheNotifyOrderNo::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllRoutingCacheNotifyOrderNo"));
std::string QueryGetAllRoutingCacheNotifyOrderNo::_baseSQL;
bool QueryGetAllRoutingCacheNotifyOrderNo::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllRoutingCacheNotifyOrderNo> g_GetAllRoutingCacheNotifyOrderNo;

const char*
QueryGetAllRoutingCacheNotifyOrderNo::getQueryName() const
{
  return "GETALLROUTINGCACHENOTIFYORDERNO";
}

void
QueryGetAllRoutingCacheNotifyOrderNo::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllRoutingCacheNotifyOrderNoSQLStatement<QueryGetAllRoutingCacheNotifyOrderNo>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLROUTINGCACHENOTIFYORDERNO");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllRoutingCacheNotifyOrderNo::findAllRoutingCacheNotifyOrderNo(
    std::vector<tse::CacheNotifyInfo>& infos,
    const orderNoList& lastOrderNos,
    DateTime& priorCutoff,
    const DateTime& cutoff,
    int pollSize)
{
  QueryGetAllRoutingCacheNotifyOrderNoSQLStatement<
      QueryGetAllRoutingCacheNotifyOrderNo>::parameterSubstitution(*this,
                                                                   lastOrderNos,
                                                                   priorCutoff,
                                                                   cutoff,
                                                                   pollSize);

  DBResultSet res(_dbAdapt);
  res.executeQuery(this);
  QueryGetAllRoutingCacheNotifyOrderNoSQLStatement<QueryGetAllRoutingCacheNotifyOrderNo>::mapResult(
      infos, res);
  res.freeResult();
  filterEntityTypes(infos);
} // findAllRoutingCacheNotifyOrderNo()

///////////////////////////////////////////////////////////
//
//  QueryGetAllSupportCacheNotifyOrderNo
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllSupportCacheNotifyOrderNo::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllSupportCacheNotifyOrderNo"));
std::string QueryGetAllSupportCacheNotifyOrderNo::_baseSQL;
bool QueryGetAllSupportCacheNotifyOrderNo::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllSupportCacheNotifyOrderNo> g_GetAllSupportCacheNotifyOrderNo;

const char*
QueryGetAllSupportCacheNotifyOrderNo::getQueryName() const
{
  return "GETALLSUPPORTCACHENOTIFYORDERNO";
}

void
QueryGetAllSupportCacheNotifyOrderNo::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllSupportCacheNotifyOrderNoSQLStatement<QueryGetAllSupportCacheNotifyOrderNo>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLSUPPORTCACHENOTIFYORDERNO");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllSupportCacheNotifyOrderNo::findAllSupportCacheNotifyOrderNo(
    std::vector<tse::CacheNotifyInfo>& infos,
    const orderNoList& lastOrderNos,
    DateTime& priorCutoff,
    const DateTime& cutoff,
    int pollSize)
{

  QueryGetAllSupportCacheNotifyOrderNoSQLStatement<
      QueryGetAllSupportCacheNotifyOrderNo>::parameterSubstitution(*this,
                                                                   lastOrderNos,
                                                                   priorCutoff,
                                                                   cutoff,
                                                                   pollSize);
  DBResultSet res(_dbAdapt);
  res.executeQuery(this);
  QueryGetAllSupportCacheNotifyOrderNoSQLStatement<QueryGetAllSupportCacheNotifyOrderNo>::mapResult(
      infos, res);
  res.freeResult();
  filterEntityTypes(infos);
} // findAllSupportCacheNotifyOrderNo()

///////////////////////////////////////////////////////////
//
//  QueryGetAllIntlCacheNotifyOrderNo
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllIntlCacheNotifyOrderNo::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllIntlCacheNotifyOrderNo"));
std::string QueryGetAllIntlCacheNotifyOrderNo::_baseSQL;
bool QueryGetAllIntlCacheNotifyOrderNo::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllIntlCacheNotifyOrderNo> g_GetAllIntlCacheNotifyOrderNo;

const char*
QueryGetAllIntlCacheNotifyOrderNo::getQueryName() const
{
  return "GETALLINTLCACHENOTIFYORDERNO";
}

void
QueryGetAllIntlCacheNotifyOrderNo::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllIntlCacheNotifyOrderNoSQLStatement<QueryGetAllIntlCacheNotifyOrderNo> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLINTLCACHENOTIFYORDERNO");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllIntlCacheNotifyOrderNo::findAllIntlCacheNotifyOrderNo(
    std::vector<tse::CacheNotifyInfo>& infos,
    const orderNoList& lastOrderNos,
    DateTime& priorCutoff,
    const DateTime& cutoff,
    int pollSize)
{
  QueryGetAllIntlCacheNotifyOrderNoSQLStatement<
      QueryGetAllIntlCacheNotifyOrderNo>::parameterSubstitution(*this,
                                                                lastOrderNos,
                                                                priorCutoff,
                                                                cutoff,
                                                                pollSize);
  DBResultSet res(_dbAdapt);
  res.executeQuery(this);
  QueryGetAllIntlCacheNotifyOrderNoSQLStatement<QueryGetAllIntlCacheNotifyOrderNo>::mapResult(infos,
                                                                                              res);
  res.freeResult();
  filterEntityTypes(infos);
} // findAllIntlCacheNotifyOrderNo()

///////////////////////////////////////////////////////////
//
//  QueryGetAllMerchandisingCacheNotifyOrderNo
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllMerchandisingCacheNotifyOrderNo::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllMerchandisingCacheNotifyOrderNo"));
std::string QueryGetAllMerchandisingCacheNotifyOrderNo::_baseSQL;
bool QueryGetAllMerchandisingCacheNotifyOrderNo::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllMerchandisingCacheNotifyOrderNo>
g_GetAllMerchandisingCacheNotifyOrderNo;

const char*
QueryGetAllMerchandisingCacheNotifyOrderNo::getQueryName() const
{
  return "GETALLMERCHANDISINGCACHENOTIFYORDERNO";
}

void
QueryGetAllMerchandisingCacheNotifyOrderNo::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllMerchandisingCacheNotifyOrderNoSQLStatement<
        QueryGetAllMerchandisingCacheNotifyOrderNo> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLMERCHANDISINGCACHENOTIFYORDERNO");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllMerchandisingCacheNotifyOrderNo::findAllMerchandisingCacheNotifyOrderNo(
    std::vector<tse::CacheNotifyInfo>& infos,
    const orderNoList& lastOrderNos,
    DateTime& priorCutoff,
    const DateTime& cutoff,
    int pollSize)
{
  QueryGetAllMerchandisingCacheNotifyOrderNoSQLStatement<
      QueryGetAllMerchandisingCacheNotifyOrderNo>::parameterSubstitution(*this,
                                                                         lastOrderNos,
                                                                         priorCutoff,
                                                                         cutoff,
                                                                         pollSize);
  DBResultSet res(_dbAdapt);
  res.executeQuery(this);
  QueryGetAllIntlCacheNotifyOrderNoSQLStatement<
      QueryGetAllMerchandisingCacheNotifyOrderNo>::mapResult(infos, res);
  res.freeResult();
  filterEntityTypes(infos);
} // findAllMerchandisingCacheNotifyOrderNo()

///////////////////////////////////////////////////////////
//
//  QueryGetAllCacheNotifyOrderNoHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllCacheNotifyOrderNoHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllCacheNotifyOrderNoHistorical"));
std::string QueryGetAllCacheNotifyOrderNoHistorical::_baseSQL;
bool QueryGetAllCacheNotifyOrderNoHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllCacheNotifyOrderNoHistorical>
g_GetAllCacheNotifyOrderNoHistorical;

const char*
QueryGetAllCacheNotifyOrderNoHistorical::getQueryName() const
{
  return "GETALLCACHENOTIFYORDERNOHISTORICAL";
}

void
QueryGetAllCacheNotifyOrderNoHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllCacheNotifyOrderNoHistoricalSQLStatement<QueryGetAllCacheNotifyOrderNoHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLCACHENOTIFYORDERNOHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllCacheNotifyOrderNoHistorical::findAllCacheNotifyOrderNoHistorical(
    std::vector<tse::CacheNotifyInfo>& infos,
    const orderNoList& lastOrderNos,
    DateTime& priorCutoff,
    const DateTime& cutoff,
    int pollSize)
{
  QueryGetAllCacheNotifyOrderNoHistoricalSQLStatement<
      QueryGetAllCacheNotifyOrderNoHistorical>::parameterSubstitution(*this,
                                                                      lastOrderNos,
                                                                      priorCutoff,
                                                                      cutoff,
                                                                      pollSize);
  DBResultSet res(_dbAdapt);
  res.executeQuery(this);
  QueryGetAllCacheNotifyOrderNoHistoricalSQLStatement<
      QueryGetAllCacheNotifyOrderNoHistorical>::mapResult(infos, res);
  res.freeResult();
  filterEntityTypes(infos);
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
// Missing Orderno Pickup Queries
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetMissingFareCacheNotifyOrderNo::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetMissingFareCacheNotifyOrderNo"));
std::string QueryGetMissingFareCacheNotifyOrderNo::_baseSQL;
bool QueryGetMissingFareCacheNotifyOrderNo::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMissingFareCacheNotifyOrderNo> g_QueryGetMissingacheNotifyOrderNo;

const char*
QueryGetMissingFareCacheNotifyOrderNo::getQueryName() const
{
  return "GETMISSINGFARECACHENOTIFYORDERNO";
}

void
QueryGetMissingFareCacheNotifyOrderNo::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMissingFareCacheNotifyOrderNoSQLStatement<QueryGetMissingFareCacheNotifyOrderNo>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMISSINGFARECACHENOTIFYORDERNO");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMissingFareCacheNotifyOrderNo::findMissingFareCacheNotifyOrderNo(
    std::vector<tse::CacheNotifyInfo>& infos, const orderNoList& missing)
{
  if (missing.size() && missing[0] != -1)
  {
    QueryGetMissingFareCacheNotifyOrderNoSQLStatement<
        QueryGetMissingFareCacheNotifyOrderNo>::parameterSubstitutionMissingOrderno(*this, missing);
    DBResultSet res(_dbAdapt);
    res.executeQuery(this);
    QueryGetMissingFareCacheNotifyOrderNoSQLStatement<
        QueryGetMissingFareCacheNotifyOrderNo>::mapResult(infos, res);
    res.freeResult();
    filterEntityTypes(infos);
  }
} // findFareMissingCacheNotifyOrderNo()

///////////////////////////////////////////////////////////
//
//  QueryGetRuleMissingCacheNotifyOrderNo
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetMissingRuleCacheNotifyOrderNo::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetMissingRuleCacheNotifyOrderNo"));
std::string QueryGetMissingRuleCacheNotifyOrderNo::_baseSQL;
bool QueryGetMissingRuleCacheNotifyOrderNo::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMissingRuleCacheNotifyOrderNo>
g_QueryGetMissingRuleCacheNotifyOrderNo;

const char*
QueryGetMissingRuleCacheNotifyOrderNo::getQueryName() const
{
  return "GETMISSINGRULECACHENOTIFYORDERNO";
}

void
QueryGetMissingRuleCacheNotifyOrderNo::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMissingRuleCacheNotifyOrderNoSQLStatement<QueryGetMissingRuleCacheNotifyOrderNo>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMISSINGRULECACHENOTIFYORDERNO");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMissingRuleCacheNotifyOrderNo::findMissingRuleCacheNotifyOrderNo(
    std::vector<tse::CacheNotifyInfo>& infos, const orderNoList& missing)
{
  if (missing.size() && missing[0] != -1)
  {
    QueryGetMissingRuleCacheNotifyOrderNoSQLStatement<
        QueryGetMissingRuleCacheNotifyOrderNo>::parameterSubstitutionMissingOrderno(*this, missing);
    DBResultSet res(_dbAdapt);
    res.executeQuery(this);
    QueryGetMissingRuleCacheNotifyOrderNoSQLStatement<
        QueryGetMissingRuleCacheNotifyOrderNo>::mapResult(infos, res);
    res.freeResult();
    filterEntityTypes(infos);
  }
} // findRuleMissingCacheNotifyOrderNo()

///////////////////////////////////////////////////////////
//
//  QueryGetRoutingMissingCacheNotifyOrderNo
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetMissingRoutingCacheNotifyOrderNo::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetMissingRoutingCacheNotifyOrderNo"));
std::string QueryGetMissingRoutingCacheNotifyOrderNo::_baseSQL;
bool QueryGetMissingRoutingCacheNotifyOrderNo::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMissingRoutingCacheNotifyOrderNo>
g_GetMissingRoutingCacheNotifyOrderNo;

const char*
QueryGetMissingRoutingCacheNotifyOrderNo::getQueryName() const
{
  return "GETMISSINGROUTINGCACHENOTIFYORDERNO";
}

void
QueryGetMissingRoutingCacheNotifyOrderNo::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMissingRoutingCacheNotifyOrderNoSQLStatement<QueryGetMissingRoutingCacheNotifyOrderNo>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMISSINGROUTINGCACHENOTIFYORDERNO");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMissingRoutingCacheNotifyOrderNo::findMissingRoutingCacheNotifyOrderNo(
    std::vector<tse::CacheNotifyInfo>& infos, const orderNoList& missing)
{
  if (missing.size() && missing[0] != -1)
  {
    QueryGetMissingRoutingCacheNotifyOrderNoSQLStatement<
        QueryGetMissingRoutingCacheNotifyOrderNo>::parameterSubstitutionMissingOrderno(*this,
                                                                                       missing);
    DBResultSet res(_dbAdapt);
    res.executeQuery(this);
    QueryGetMissingRoutingCacheNotifyOrderNoSQLStatement<
        QueryGetMissingRoutingCacheNotifyOrderNo>::mapResult(infos, res);
    res.freeResult();
    filterEntityTypes(infos);
  }
} // findRoutingMissingCacheNotifyOrderNo()

///////////////////////////////////////////////////////////
//
//  QueryGetSupportMissingCacheNotifyOrderNo
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetMissingSupportCacheNotifyOrderNo::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetMissingSupportCacheNotifyOrderNo"));
std::string QueryGetMissingSupportCacheNotifyOrderNo::_baseSQL;
bool QueryGetMissingSupportCacheNotifyOrderNo::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMissingSupportCacheNotifyOrderNo>
g_QueryGetMissingSupportCacheNotifyOrderNo;

const char*
QueryGetMissingSupportCacheNotifyOrderNo::getQueryName() const
{
  return "GETMISSINGSUPPORTCACHENOTIFYORDERNO";
}

void
QueryGetMissingSupportCacheNotifyOrderNo::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMissingSupportCacheNotifyOrderNoSQLStatement<QueryGetMissingSupportCacheNotifyOrderNo>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMISSINGSUPPORTCACHENOTIFYORDERNO");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMissingSupportCacheNotifyOrderNo::findMissingSupportCacheNotifyOrderNo(
    std::vector<tse::CacheNotifyInfo>& infos, const orderNoList& missing)
{
  if (missing.size() && missing[0] != -1)
  {
    QueryGetMissingSupportCacheNotifyOrderNoSQLStatement<
        QueryGetMissingSupportCacheNotifyOrderNo>::parameterSubstitutionMissingOrderno(*this,
                                                                                       missing);
    DBResultSet res(_dbAdapt);
    res.executeQuery(this);
    QueryGetMissingSupportCacheNotifyOrderNoSQLStatement<
        QueryGetMissingSupportCacheNotifyOrderNo>::mapResult(infos, res);
    res.freeResult();
    filterEntityTypes(infos);
  }
} // findSupportMissingCacheNotifyOrderNo()

///////////////////////////////////////////////////////////
//
//  QueryGetIntlMissingCacheNotifyOrderNo
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetMissingIntlCacheNotifyOrderNo::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetMissingIntlCacheNotifyOrderNo"));
std::string QueryGetMissingIntlCacheNotifyOrderNo::_baseSQL;
bool QueryGetMissingIntlCacheNotifyOrderNo::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMissingIntlCacheNotifyOrderNo>
g_QueryGetMissingIntlCacheNotifyOrderNo;

const char*
QueryGetMissingIntlCacheNotifyOrderNo::getQueryName() const
{
  return "GETMISSINGINTLCACHENOTIFYORDERNO";
}

void
QueryGetMissingIntlCacheNotifyOrderNo::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMissingIntlCacheNotifyOrderNoSQLStatement<QueryGetMissingIntlCacheNotifyOrderNo>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMISSINGINTLCACHENOTIFYORDERNO");
    substTableDef(&_baseSQL);

    _isInitialized = true;
  }
} // initialize()

void
QueryGetMissingIntlCacheNotifyOrderNo::findMissingIntlCacheNotifyOrderNo(
    std::vector<tse::CacheNotifyInfo>& infos, const orderNoList& missing)
{
  if (missing.size() && missing[0] != -1)
  {
    QueryGetMissingIntlCacheNotifyOrderNoSQLStatement<
        QueryGetMissingIntlCacheNotifyOrderNo>::parameterSubstitutionMissingOrderno(*this, missing);
    DBResultSet res(_dbAdapt);
    res.executeQuery(this);
    QueryGetMissingIntlCacheNotifyOrderNoSQLStatement<
        QueryGetMissingIntlCacheNotifyOrderNo>::mapResult(infos, res);
    res.freeResult();
    filterEntityTypes(infos);
  }

} // findIntlMissingCacheNotifyOrderNo()

///////////////////////////////////////////////////////////
//
//  QueryGetMerchandisingMissingCacheNotifyOrderNo
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetMissingMerchandisingCacheNotifyOrderNo::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetMissingMerchandisingCacheNotifyOrderNo"));
std::string QueryGetMissingMerchandisingCacheNotifyOrderNo::_baseSQL;
bool QueryGetMissingMerchandisingCacheNotifyOrderNo::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMissingMerchandisingCacheNotifyOrderNo>
g_GetMissingMerchandisingCacheNotifyOrderNo;

const char*
QueryGetMissingMerchandisingCacheNotifyOrderNo::getQueryName() const
{
  return "GETMISSINGMERCHANDISINGCACHENOTIFYORDERNO";
}

void
QueryGetMissingMerchandisingCacheNotifyOrderNo::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMissingMerchandisingCacheNotifyOrderNoSQLStatement<
        QueryGetMissingMerchandisingCacheNotifyOrderNo> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMISSINGMERCHANDISINGCACHENOTIFYORDERNO");
    substTableDef(&_baseSQL);

    _isInitialized = true;
  }
} // initialize()

void
QueryGetMissingMerchandisingCacheNotifyOrderNo::findMissingMerchandisingCacheNotifyOrderNo(
    std::vector<tse::CacheNotifyInfo>& infos, const orderNoList& missing)
{
  if (missing.size() && missing[0] != -1)
  {
    QueryGetMissingMerchandisingCacheNotifyOrderNoSQLStatement<
        QueryGetMissingMerchandisingCacheNotifyOrderNo>::
        parameterSubstitutionMissingOrderno(*this, missing);
    DBResultSet res(_dbAdapt);
    res.executeQuery(this);
    QueryGetMissingIntlCacheNotifyOrderNoSQLStatement<
        QueryGetMissingMerchandisingCacheNotifyOrderNo>::mapResult(infos, res);
    res.freeResult();
    filterEntityTypes(infos);
  }

} // findMissingMerchandisingCacheNotifyOrderNo()

///////////////////////////////////////////////////////////
//
//  QueryGetMissingCacheNotifyOrderNoHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetMissingCacheNotifyOrderNoHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetMissingCacheNotifyOrderNoHistorical"));
std::string QueryGetMissingCacheNotifyOrderNoHistorical::_baseSQL;
bool QueryGetMissingCacheNotifyOrderNoHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMissingCacheNotifyOrderNoHistorical>
g_GetMissingCacheNotifyOrderNoHistorical;

const char*
QueryGetMissingCacheNotifyOrderNoHistorical::getQueryName() const
{
  return "GETMISSINGCACHENOTIFYORDERNOHISTORICAL";
}

void
QueryGetMissingCacheNotifyOrderNoHistorical::initialize()
{
  if (!_isInitialized)
  {
    {
      QueryGetMissingCacheNotifyOrderNoHistoricalSQLStatement<
          QueryGetMissingCacheNotifyOrderNoHistorical> sqlStatement;
      _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMISSINGCACHENOTIFYORDERNOHISTORICAL");
      substTableDef(&_baseSQL);
    }
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMissingCacheNotifyOrderNoHistorical::findMissingCacheNotifyOrderNoHistorical(
    std::vector<tse::CacheNotifyInfo>& infos, const orderNoList& missing)
{
  if (missing.size() && missing[0] != -1)
  {
    QueryGetMissingCacheNotifyOrderNoHistoricalSQLStatement<
        QueryGetMissingCacheNotifyOrderNoHistorical>::parameterSubstitutionMissingOrderno(*this,
                                                                                          missing);
    DBResultSet res(_dbAdapt);
    res.executeQuery(this);
    QueryGetMissingCacheNotifyOrderNoHistoricalSQLStatement<
        QueryGetMissingCacheNotifyOrderNoHistorical>::mapResult(infos, res);
    res.freeResult();
    filterEntityTypes(infos);
  }
}
}
