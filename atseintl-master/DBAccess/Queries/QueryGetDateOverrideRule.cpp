//----------------------------------------------------------------------------
//  File:           QueryGetDateOverrideRule.cpp
//  Description:    QueryGetDateOverrideRule
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
#include "DBAccess/Queries/QueryGetDateOverrideRule.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetDateOverrideRuleSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetDateOverrideRule::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetDateOverrideRule"));
std::string QueryGetDateOverrideRule::_baseSQL;
bool QueryGetDateOverrideRule::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetDateOverrideRule> g_GetDateOverrideRule;

const char*
QueryGetDateOverrideRule::getQueryName() const
{
  return "GETDATEOVERRIDERULE";
}

void
QueryGetDateOverrideRule::initialize()
{
  if (!_isInitialized)
  {
    QueryGetDateOverrideRuleSQLStatement<QueryGetDateOverrideRule> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETDATEOVERRIDERULE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetDateOverrideRule::findDateOverrideRuleItem(
    std::vector<DateOverrideRuleItem*>& dateOverrides, const VendorCode& vendor, int itemNumber)
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
    dateOverrides.push_back(QueryGetDateOverrideRuleSQLStatement<
        QueryGetDateOverrideRule>::mapRowToDateOverrideRuleItem(row));
  }
  LOG4CXX_INFO(_logger,
               "GETDATEOVERRIDERULE: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                 << " (" << stopCPU() << ")");
  res.freeResult();
} // findDateOverrideRuleItem()

///////////////////////////////////////////////////////////
//  QueryGetDateOverrideRuleHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetDateOverrideRuleHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetDateOverrideRuleHistorical"));
std::string QueryGetDateOverrideRuleHistorical::_baseSQL;
bool QueryGetDateOverrideRuleHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetDateOverrideRuleHistorical> g_GetDateOverrideRuleHistorical;

const char*
QueryGetDateOverrideRuleHistorical::getQueryName() const
{
  return "GETDATEOVERRIDERULEHISTORICAL";
}

void
QueryGetDateOverrideRuleHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetDateOverrideRuleHistoricalSQLStatement<QueryGetDateOverrideRuleHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETDATEOVERRIDERULEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetDateOverrideRuleHistorical::findDateOverrideRuleItem(
    std::vector<DateOverrideRuleItem*>& dateOverrides,
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
    dateOverrides.push_back(QueryGetDateOverrideRuleHistoricalSQLStatement<
        QueryGetDateOverrideRuleHistorical>::mapRowToDateOverrideRuleItem(row));
  }
  LOG4CXX_INFO(_logger,
               "GETDATEOVERRIDERULEHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findDateOverrideRuleItem()
}
