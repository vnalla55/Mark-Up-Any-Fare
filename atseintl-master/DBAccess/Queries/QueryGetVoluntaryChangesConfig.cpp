//----------------------------------------------------------------------------
//  File:           QueryGetVoluntaryChangesConfig.cpp
//  Description:    QueryGetVoluntaryChangesConfig
//  Created:        10/28/2008
// Authors:         Dean Van Decker
//
//  Updates:
//
// � 2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetVoluntaryChangesConfig.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetVoluntaryChangesConfigSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetVoluntaryChangesConfig::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetVoluntaryChangesConfig"));
std::string QueryGetVoluntaryChangesConfig::_baseSQL;
bool QueryGetVoluntaryChangesConfig::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetVoluntaryChangesConfig> g_GetVoluntaryChangesConfig;

const char*
QueryGetVoluntaryChangesConfig::getQueryName() const
{
  return "GETVOLUNTARYCHANGESCONFIG";
}

void
QueryGetVoluntaryChangesConfig::initialize()
{
  if (!_isInitialized)
  {
    QueryGetVoluntaryChangesConfigSQLStatement<QueryGetVoluntaryChangesConfig> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETVOLUNTARYCHANGESCONFIG");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetVoluntaryChangesConfig::findVoluntaryChangesConfig(
    std::vector<tse::VoluntaryChangesConfig*>& voluntaryChangesConfig, const CarrierCode& carrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, carrier);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    voluntaryChangesConfig.push_back(QueryGetVoluntaryChangesConfigSQLStatement<
        QueryGetVoluntaryChangesConfig>::mapRowToVoluntaryChangesConfig(row));
  }
  LOG4CXX_INFO(_logger,
               "GETVOLUNTARYCHANGESCONFIG: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                       << " (" << stopCPU() << ")");
  res.freeResult();
} // findVoluntaryChangesConfig()

///////////////////////////////////////////////////////////
//
//  QueryGetAllVoluntaryChangesConfig
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllVoluntaryChangesConfig::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllVoluntaryChangesConfig"));
std::string QueryGetAllVoluntaryChangesConfig::_baseSQL;
bool QueryGetAllVoluntaryChangesConfig::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllVoluntaryChangesConfig> g_GetAllVoluntaryChangesConfig;

const char*
QueryGetAllVoluntaryChangesConfig::getQueryName() const
{
  return "GETALLVOLUNTARYCHANGESCONFIG";
}

void
QueryGetAllVoluntaryChangesConfig::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllVoluntaryChangesConfigSQLStatement<QueryGetAllVoluntaryChangesConfig> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLVOLUNTARYCHANGESCONFIG");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllVoluntaryChangesConfig::findAllVoluntaryChangesConfigs(
    std::vector<tse::VoluntaryChangesConfig*>& voluntaryChangesConfig)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    voluntaryChangesConfig.push_back(QueryGetAllVoluntaryChangesConfigSQLStatement<
        QueryGetAllVoluntaryChangesConfig>::mapRowToVoluntaryChangesConfig(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLVOLUNTARYCHANGESCONFIG: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllVoluntaryChangesConfig()
}
