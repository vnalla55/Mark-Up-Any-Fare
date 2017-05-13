//----------------------------------------------------------------------------
//  File:           QueryGetOpenJawRule.cpp
//  Description:    QueryGetOpenJawRule
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
#include "DBAccess/Queries/QueryGetOpenJawRule.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetOpenJawRuleSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetOpenJawRule::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetOpenJawRule"));
std::string QueryGetOpenJawRule::_baseSQL;
bool QueryGetOpenJawRule::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetOpenJawRule> g_GetOpenJawRule;

const char*
QueryGetOpenJawRule::getQueryName() const
{
  return "GETOPENJAWRULE";
}

void
QueryGetOpenJawRule::initialize()
{
  if (!_isInitialized)
  {
    QueryGetOpenJawRuleSQLStatement<QueryGetOpenJawRule> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETOPENJAWRULE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetOpenJawRule::findOpenJawRule(std::vector<OpenJawRule*>& openJaws,
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
    openJaws.push_back(
        QueryGetOpenJawRuleSQLStatement<QueryGetOpenJawRule>::mapRowToOpenJawRule(row));
  }
  LOG4CXX_INFO(_logger,
               "GETOPENJAWRULE: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                            << stopCPU() << ")");
  res.freeResult();
} // findOpenJawRule()

///////////////////////////////////////////////////////////
//  QueryGetOpenJawRuleHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetOpenJawRuleHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetOpenJawRuleHistorical"));
std::string QueryGetOpenJawRuleHistorical::_baseSQL;
bool QueryGetOpenJawRuleHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetOpenJawRuleHistorical> g_GetOpenJawRuleHistorical;

const char*
QueryGetOpenJawRuleHistorical::getQueryName() const
{
  return "GETOPENJAWRULEHISTORICAL";
}

void
QueryGetOpenJawRuleHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetOpenJawRuleHistoricalSQLStatement<QueryGetOpenJawRuleHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETOPENJAWRULEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetOpenJawRuleHistorical::findOpenJawRule(std::vector<OpenJawRule*>& openJaws,
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
    openJaws.push_back(QueryGetOpenJawRuleHistoricalSQLStatement<
        QueryGetOpenJawRuleHistorical>::mapRowToOpenJawRule(row));
  }
  LOG4CXX_INFO(_logger,
               "GETOPENJAWRULEHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ")");
  res.freeResult();
} // findOpenJawRule()
}
