//----------------------------------------------------------------------------
//  File:           QueryGetGeoRule.cpp
//  Description:    QueryGetGeoRule
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
#include "DBAccess/Queries/QueryGetGeoRule.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetGeoRuleSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetGeoRule::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetGeoRule"));
std::string QueryGetGeoRule::_baseSQL;
bool QueryGetGeoRule::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetGeoRule> g_GetGeoRule;

const char*
QueryGetGeoRule::getQueryName() const
{
  return "GETGEORULE";
}

void
QueryGetGeoRule::initialize()
{
  if (!_isInitialized)
  {
    QueryGetGeoRuleSQLStatement<QueryGetGeoRule> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETGEORULE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetGeoRule::findGeoRuleItem(std::vector<GeoRuleItem*>& geoRules,
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
    geoRules.push_back(QueryGetGeoRuleSQLStatement<QueryGetGeoRule>::mapRowToGeoRuleItem(row));
  }
  LOG4CXX_INFO(_logger,
               "GETGEORULE: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                        << stopCPU() << ")");
  res.freeResult();
} // findGeoRuleItem()

///////////////////////////////////////////////////////////
//  QueryGetGeoRuleHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetGeoRuleHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetGeoRuleHistorical"));
std::string QueryGetGeoRuleHistorical::_baseSQL;
bool QueryGetGeoRuleHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetGeoRuleHistorical> g_GetGeoRuleHistorical;

const char*
QueryGetGeoRuleHistorical::getQueryName() const
{
  return "GETGEORULEHISTORICAL";
}

void
QueryGetGeoRuleHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetGeoRuleHistoricalSQLStatement<QueryGetGeoRuleHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETGEORULEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetGeoRuleHistorical::findGeoRuleItem(std::vector<GeoRuleItem*>& geoRules,
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
    geoRules.push_back(
        QueryGetGeoRuleHistoricalSQLStatement<QueryGetGeoRuleHistorical>::mapRowToGeoRuleItem(row));
  }
  LOG4CXX_INFO(_logger,
               "GETGEORULEHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                  << " (" << stopCPU() << ")");
  res.freeResult();
} // findGeoRuleItem()
}
