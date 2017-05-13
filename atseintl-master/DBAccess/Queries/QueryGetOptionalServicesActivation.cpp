//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetOptionalServicesActivation.h"

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetOptionalServicesActivationSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetOptionalServicesActivation::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetOptionalServicesActivation"));
std::string QueryGetOptionalServicesActivation::_baseSQL;
bool QueryGetOptionalServicesActivation::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetOptionalServicesActivation> g_GetOptServicesActivation;

const char*
QueryGetOptionalServicesActivation::getQueryName() const
{
  return "GETOPTIONALSERVICESACTIVATION";
}

void
QueryGetOptionalServicesActivation::initialize()
{
  if (!_isInitialized)
  {
    QueryGetOptionalServicesActivationSQLStatement<QueryGetOptionalServicesActivation> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETOPTIONALSERVICESACTIVATION");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetOptionalServicesActivation::findOptionalServicesActivationInfo(
    std::vector<OptionalServicesActivationInfo*>& optSrvActivation,
    Indicator crs,
    const UserApplCode& userCode,
    const std::string& application)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, crs);
  substParm(userCode, 2);
  substParm(application, 3);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    optSrvActivation.push_back(QueryGetOptionalServicesActivationSQLStatement<
        QueryGetOptionalServicesActivation>::mapRowToOptServicesActivationInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETOPTIONALSERVICESACTIVATION: NumRows "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ") mSecs");
  res.freeResult();
}

///////////////////////////////////////////////////////////
//  QueryGetOptionalServicesActivationHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetOptionalServicesActivationHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.QueryGetOptionalServicesActivationHistorical"));
std::string QueryGetOptionalServicesActivationHistorical::_baseSQL;
bool QueryGetOptionalServicesActivationHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetOptionalServicesActivationHistorical>
g_GetOptServicesActivationHistorical;

const char*
QueryGetOptionalServicesActivationHistorical::getQueryName() const
{
  return "GETOPTIONALSERVICESACTIVATIONHISTORICAL";
}

void
QueryGetOptionalServicesActivationHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetOptionalServicesActivationHistoricalSQLStatement<
        QueryGetOptionalServicesActivationHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETOPTIONALSERVICESACTIVATIONHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetOptionalServicesActivationHistorical::findOptionalServicesActivationInfo(
    std::vector<OptionalServicesActivationInfo*>& optSrvActivation,
    Indicator crs,
    const UserApplCode& userCode,
    const std::string& application,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, crs);
  substParm(userCode, 2);
  substParm(application, 3);
  substParm(4, startDate);
  substParm(5, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    optSrvActivation.push_back(QueryGetOptionalServicesActivationHistoricalSQLStatement<
        QueryGetOptionalServicesActivationHistorical>::mapRowToOptServicesActivationInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETOPTIONALSERVICESACTIVATIONHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
}
} // tse
