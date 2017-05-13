//----------------------------------------------------------------------------
//  File:           QueryGetFlightAppRule.cpp
//  Description:    QueryGetFlightAppRule
//  Created:        5/24/2006
// Authors:         Mike Lillis
//
//  Updates:
//
// � 2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetFlightAppRule.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFlightAppRuleSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetFlightAppRule::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetFlightAppRule"));
std::string QueryGetFlightAppRule::_baseSQL;
bool QueryGetFlightAppRule::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFlightAppRule> g_GetFlightAppRule;

const char*
QueryGetFlightAppRule::getQueryName() const
{
  return "GETFLIGHTAPPRULE";
}

void
QueryGetFlightAppRule::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFlightAppRuleSQLStatement<QueryGetFlightAppRule> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFLIGHTAPPRULE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetFlightAppRule::findFlightAppRule(std::vector<FlightAppRule*>& flightApps,
                                         const VendorCode& vendor,
                                         int itemNumber)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char itemStr[15];
  sprintf(itemStr, "%d", itemNumber);

  substParm(vendor, 1);
  substParm(itemStr, 2);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    flightApps.push_back(
        QueryGetFlightAppRuleSQLStatement<QueryGetFlightAppRule>::mapRowToFlightAppRule(row));
  }
  LOG4CXX_INFO(_logger,
               "GETFLIGHTAPPRULE: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                              << stopCPU() << ")");
  res.freeResult();
} // findFlightAppRule()

///////////////////////////////////////////////////////////
//  QueryGetFlightAppRuleHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetFlightAppRuleHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetFlightAppRuleHistorical"));
std::string QueryGetFlightAppRuleHistorical::_baseSQL;
bool QueryGetFlightAppRuleHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFlightAppRuleHistorical> g_GetFlightAppRuleHistorical;

const char*
QueryGetFlightAppRuleHistorical::getQueryName() const
{
  return "GETFLIGHTAPPRULEHISTORICAL";
}

void
QueryGetFlightAppRuleHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFlightAppRuleHistoricalSQLStatement<QueryGetFlightAppRuleHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFLIGHTAPPRULEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetFlightAppRuleHistorical::findFlightAppRule(std::vector<FlightAppRule*>& flightApps,
                                                   const VendorCode& vendor,
                                                   int itemNumber,
                                                   const DateTime& startDate,
                                                   const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char itemStr[15];
  sprintf(itemStr, "%d", itemNumber);

  substParm(vendor, 1);
  substParm(itemStr, 2);
  substParm(3, startDate);
  substParm(4, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    flightApps.push_back(QueryGetFlightAppRuleHistoricalSQLStatement<
        QueryGetFlightAppRuleHistorical>::mapRowToFlightAppRule(row));
  }
  LOG4CXX_INFO(_logger,
               "GETFLIGHTAPPRULEHISTORICAL: NumRows = " << res.numRows() << " Time = "
                                                        << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findFlightAppRule()
}
