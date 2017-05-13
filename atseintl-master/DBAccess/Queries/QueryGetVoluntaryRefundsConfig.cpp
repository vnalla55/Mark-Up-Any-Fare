//-----------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly
// prohibited.
//-----------------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetVoluntaryRefundsConfig.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetVoluntaryRefundsConfigSQLStatement.h"

namespace tse
{

log4cxx::LoggerPtr
QueryGetVoluntaryRefundsConfig::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetVoluntaryRefundsConfig"));

std::string QueryGetVoluntaryRefundsConfig::_baseSQL;

bool QueryGetVoluntaryRefundsConfig::_isInitialized = false;

SQLQueryInitializerHelper<QueryGetVoluntaryRefundsConfig> g_GetVoluntaryRefundsConfig;

namespace
{
typedef QueryGetVoluntaryRefundsConfigSQLStatement<QueryGetVoluntaryRefundsConfig> SqlStatement;
typedef QueryGetAllVoluntaryRefundsConfigSQLStatement<QueryGetAllVoluntaryRefundsConfig>
SqlStatementAll;
}

void
QueryGetVoluntaryRefundsConfig::initialize()
{
  if (!_isInitialized)
  {
    SqlStatement sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETVOLUNTARYREFUNDSCONFIG");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetVoluntaryRefundsConfig::find(std::vector<VoluntaryRefundsConfig*>& lst,
                                     const CarrierCode& carrier)
{
  DBResultSet res(_dbAdapt);

  substParm(1, carrier);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  lst.reserve(res.numRows());
  while (Row* row = res.nextRow())
  {
    lst.push_back(SqlStatement::map(row));
  }
  LOG4CXX_INFO(_logger,
               "GETVOLUNTARYREFUNDSCONFIG: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                       << " (" << stopCPU() << ")");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetAllVoluntaryRefundsConfig::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllVoluntaryRefundsConfig"));

std::string QueryGetAllVoluntaryRefundsConfig::_baseSQL;

bool QueryGetAllVoluntaryRefundsConfig::_isInitialized = false;

SQLQueryInitializerHelper<QueryGetAllVoluntaryRefundsConfig> g_GetAllVoluntaryRefundsConfig;

void
QueryGetAllVoluntaryRefundsConfig::initialize()
{
  if (!_isInitialized)
  {
    SqlStatementAll sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLVOLUNTARYREFUNDSCONFIG");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetAllVoluntaryRefundsConfig::find(std::vector<VoluntaryRefundsConfig*>& lst)
{
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  lst.reserve(lst.size() + res.numRows());
  while (Row* row = res.nextRow())
  {
    lst.push_back(SqlStatementAll::map(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLVOLUNTARYREFUNDSCONFIG: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
}
}
