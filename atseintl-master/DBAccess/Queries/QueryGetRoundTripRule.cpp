//----------------------------------------------------------------------------
//  File:           QueryGetRoundTripRule.cpp
//  Description:    QueryGetRoundTripRule
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
#include "DBAccess/Queries/QueryGetRoundTripRule.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetRoundTripRuleSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetRoundTripRule::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetRoundTripRule"));
std::string QueryGetRoundTripRule::_baseSQL;
bool QueryGetRoundTripRule::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetRoundTripRule> g_GetRoundTripRule;

const char*
QueryGetRoundTripRule::getQueryName() const
{
  return "GETROUNDTRIPRULE";
}

void
QueryGetRoundTripRule::initialize()
{
  if (!_isInitialized)
  {
    QueryGetRoundTripRuleSQLStatement<QueryGetRoundTripRule> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETROUNDTRIPRULE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetRoundTripRule::findRoundTripRuleItem(std::vector<RoundTripRuleItem*>& ctRules,
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
    ctRules.push_back(
        QueryGetRoundTripRuleSQLStatement<QueryGetRoundTripRule>::mapRowToRoundTripRuleItem(row));
  }
  LOG4CXX_INFO(_logger,
               "GETROUNDTRIPRULE: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                              << stopCPU() << ")");
  res.freeResult();
} // findRoundTripRuleItem()

///////////////////////////////////////////////////////////
//  QueryGetRoundTripRuleHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetRoundTripRuleHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetRoundTripRuleHistorical"));
std::string QueryGetRoundTripRuleHistorical::_baseSQL;
bool QueryGetRoundTripRuleHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetRoundTripRuleHistorical> g_GetRoundTripRuleHistorical;

const char*
QueryGetRoundTripRuleHistorical::getQueryName() const
{
  return "GETROUNDTRIPRULEHISTORICAL";
}

void
QueryGetRoundTripRuleHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetRoundTripRuleHistoricalSQLStatement<QueryGetRoundTripRuleHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETROUNDTRIPRULEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetRoundTripRuleHistorical::findRoundTripRuleItem(std::vector<RoundTripRuleItem*>& ctRules,
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
    ctRules.push_back(QueryGetRoundTripRuleHistoricalSQLStatement<
        QueryGetRoundTripRuleHistorical>::mapRowToRoundTripRuleItem(row));
  }
  LOG4CXX_INFO(_logger,
               "GETROUNDTRIPRULEHISTORICAL: NumRows = " << res.numRows() << " Time = "
                                                        << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findRoundTripRuleItem()
}
