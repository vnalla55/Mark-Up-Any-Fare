//----------------------------------------------------------------------------
//  File:           QueryGetTariffRuleRest.cpp
//  Description:    QueryGetTariffRuleRest
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
#include "DBAccess/Queries/QueryGetTariffRuleRest.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetTariffRuleRestSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetTariffRuleRest::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTariffRuleRest"));
std::string QueryGetTariffRuleRest::_baseSQL;
bool QueryGetTariffRuleRest::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTariffRuleRest> g_GetTariffRuleRest;

const char*
QueryGetTariffRuleRest::getQueryName() const
{
  return "GETTARIFFRULEREST";
}

void
QueryGetTariffRuleRest::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTariffRuleRestSQLStatement<QueryGetTariffRuleRest> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTARIFFRULEREST");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTariffRuleRest::findTariffRuleRest(std::vector<TariffRuleRest*>& tariffRs,
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
    tariffRs.push_back(
        QueryGetTariffRuleRestSQLStatement<QueryGetTariffRuleRest>::mapRowToTariffRuleRest(row));
  }
  LOG4CXX_INFO(_logger,
               "GETTARIFFRULEREST: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                               << stopCPU() << ")");
  res.freeResult();
} // findTariffRuleRest()

///////////////////////////////////////////////////////////
//  QueryGetTariffRuleRestHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetTariffRuleRestHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetTariffRuleRestHistorical"));
std::string QueryGetTariffRuleRestHistorical::_baseSQL;
bool QueryGetTariffRuleRestHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTariffRuleRestHistorical> g_GetTariffRuleRestHistorical;

const char*
QueryGetTariffRuleRestHistorical::getQueryName() const
{
  return "GETTARIFFRULERESTHISTORICAL";
}

void
QueryGetTariffRuleRestHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTariffRuleRestHistoricalSQLStatement<QueryGetTariffRuleRestHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTARIFFRULERESTHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTariffRuleRestHistorical::findTariffRuleRest(std::vector<TariffRuleRest*>& tariffRs,
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
    tariffRs.push_back(QueryGetTariffRuleRestHistoricalSQLStatement<
        QueryGetTariffRuleRestHistorical>::mapRowToTariffRuleRest(row));
  }
  LOG4CXX_INFO(_logger,
               "GETTARIFFRULERESTHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findTariffRuleRest()
}
