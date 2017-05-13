//----------------------------------------------------------------------------
//  File:           QueryGetGeneralRule.cpp
//  Description:    QueryGetGeneralRule
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
#include "DBAccess/Queries/QueryGetGeneralRule.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetGeneralRuleSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetGeneralRule::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetGeneralRule"));
std::string QueryGetGeneralRule::_baseSQL;
bool QueryGetGeneralRule::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetGeneralRule> g_GetGeneralRule;

const char*
QueryGetGeneralRule::getQueryName() const
{
  return "GETGENERALRULE";
}

void
QueryGetGeneralRule::initialize()
{
  if (!_isInitialized)
  {
    QueryGetGeneralRuleSQLStatement<QueryGetGeneralRule> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETGENERALRULE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetGeneralRule::findGeneralRuleApp(std::vector<tse::GeneralRuleApp*>& generalRules,
                                        const VendorCode& vendor,
                                        const CarrierCode& carrier,
                                        int ruleTariff,
                                        const RuleNumber& ruleNo)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char ruleTarStr[15];
  sprintf(ruleTarStr, "%d", ruleTariff);

  substParm(vendor, 1);
  substParm(carrier, 2);
  substParm(ruleTarStr, 3);
  substParm(ruleNo, 4);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    generalRules.push_back(
        QueryGetGeneralRuleSQLStatement<QueryGetGeneralRule>::mapRowToGeneralRuleApp(row));
  }
  LOG4CXX_INFO(_logger,
               "GETGENERALRULE: NumRows " << res.numRows() << " Time = " << stopTimer() << " ("
                                          << stopCPU() << ") mSecs");
  res.freeResult();
} // findGeneralRuleApp()

///////////////////////////////////////////////////////////
//  QueryGetGeneralRuleHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetGeneralRuleHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetGeneralRuleHistorical"));
std::string QueryGetGeneralRuleHistorical::_baseSQL;
bool QueryGetGeneralRuleHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetGeneralRuleHistorical> g_GeGeneralRuleHistorical;

const char*
QueryGetGeneralRuleHistorical::getQueryName() const
{
  return "GETGENERALRULEHISTORICAL";
}

void
QueryGetGeneralRuleHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetGeneralRuleHistoricalSQLStatement<QueryGetGeneralRuleHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETGENERALRULEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetGeneralRuleHistorical::findGeneralRuleApp(std::vector<tse::GeneralRuleApp*>& generalRules,
                                                  const VendorCode& vendor,
                                                  const CarrierCode& carrier,
                                                  int ruleTariff,
                                                  const RuleNumber& ruleNo,
                                                  const DateTime& startDate,
                                                  const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char ruleTarStr[15];
  sprintf(ruleTarStr, "%d", ruleTariff);

  substParm(vendor, 1);
  substParm(carrier, 2);
  substParm(ruleTarStr, 3);
  substParm(ruleNo, 4);
  substParm(5, startDate);
  substParm(6, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    generalRules.push_back(QueryGetGeneralRuleHistoricalSQLStatement<
        QueryGetGeneralRuleHistorical>::mapRowToGeneralRuleApp(row));
  }
  LOG4CXX_INFO(_logger,
               "GETGENERALRULEHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ")");
  res.freeResult();
} // findGeneralRuleApp()
}
