//----------------------------------------------------------------------------
//  File:           QueryGetRuleAppl.cpp
//  Description:    QueryGetRuleAppl
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
#include "DBAccess/Queries/QueryGetRuleAppl.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetRuleApplSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetRuleAppl::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetRuleAppl"));
std::string QueryGetRuleAppl::_baseSQL;
bool QueryGetRuleAppl::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetRuleAppl> g_GetRuleAppl;

const char*
QueryGetRuleAppl::getQueryName() const
{
  return "GETRULEAPPL";
}

void
QueryGetRuleAppl::initialize()
{
  if (!_isInitialized)
  {
    QueryGetRuleApplSQLStatement<QueryGetRuleAppl> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETRULEAPPL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetRuleAppl::findRuleApplication(std::vector<tse::RuleApplication*>& ruleAppl,
                                      const VendorCode& vendor,
                                      int itemNo)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char itemStr[15];
  sprintf(itemStr, "%d", itemNo);

  substParm(vendor, 1);
  substParm(itemStr, 2);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  RuleApplication* prev = nullptr;
  while ((row = res.nextRow()))
  {
    RuleApplication* rec =
        QueryGetRuleApplSQLStatement<QueryGetRuleAppl>::mapRowToRuleApplication(row, prev);
    if (prev != rec)
    {
      ruleAppl.push_back(rec);
      prev = rec;
    }
  }
  LOG4CXX_INFO(_logger,
               "GETRULEAPPL: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                         << stopCPU() << ")");
  res.freeResult();
} // findRuleAppl()

///////////////////////////////////////////////////////////
//  QueryGetRuleApplHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetRuleApplHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetRuleApplHistorical"));
std::string QueryGetRuleApplHistorical::_baseSQL;
bool QueryGetRuleApplHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetRuleApplHistorical> g_GetRuleApplHistorical;

const char*
QueryGetRuleApplHistorical::getQueryName() const
{
  return "GETRULEAPPLHISTORICAL";
}

void
QueryGetRuleApplHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetRuleApplHistoricalSQLStatement<QueryGetRuleApplHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETRULEAPPLHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetRuleApplHistorical::findRuleApplication(std::vector<tse::RuleApplication*>& ruleAppl,
                                                const VendorCode& vendor,
                                                int itemNo,
                                                const DateTime& startDate,
                                                const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char itemStr[15];
  sprintf(itemStr, "%d", itemNo);

  substParm(vendor, 1);
  substParm(itemStr, 2);
  substParm(3, startDate);
  substParm(4, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  RuleApplication* prev = nullptr;
  while ((row = res.nextRow()))
  {
    RuleApplication* rec =
        QueryGetRuleApplHistoricalSQLStatement<QueryGetRuleApplHistorical>::mapRowToRuleApplication(
            row, prev);
    if (prev != rec)
    {
      ruleAppl.push_back(rec);
      prev = rec;
    }
  }
  LOG4CXX_INFO(_logger,
               "GETRULEAPPLHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                   << " (" << stopCPU() << ")");
  res.freeResult();
} // findRuleApplication()
}
