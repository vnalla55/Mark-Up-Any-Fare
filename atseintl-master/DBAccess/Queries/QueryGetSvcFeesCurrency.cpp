//----------------------------------------------------------------------------
//  File:           QueryGetSvcFeesCurrency.cpp
//  Description:    QuerySvcFeesCurrency
//  Created:        11/10/2009
// Authors:
//
//  Updates:
//
//  2009, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetSvcFeesCurrency.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetSvcFeesCurrencySQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetSvcFeesCurrency::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetSvcFeesCurrency"));
std::string QueryGetSvcFeesCurrency::_baseSQL;
bool QueryGetSvcFeesCurrency::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSvcFeesCurrency> g_GetSvcFeesCurrency;

const char*
QueryGetSvcFeesCurrency::getQueryName() const
{
  return "GETSVCFEESCURRENCY";
}

void
QueryGetSvcFeesCurrency::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSvcFeesCurrencySQLStatement<QueryGetSvcFeesCurrency> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSVCFEESCURRENCY");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetSvcFeesCurrency::findSvcFeesCurrencyInfo(std::vector<tse::SvcFeesCurrencyInfo*>& currency,
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
  while ((row = res.nextRow()))
  {
    currency.push_back(
        QueryGetSvcFeesCurrencySQLStatement<QueryGetSvcFeesCurrency>::mapRowToSvcFeesCurrencyInfo(
            row));
  }
  LOG4CXX_INFO(_logger,
               "GETSVCFEESCURRENCY: NumRows " << res.numRows() << " Time = " << stopTimer() << " ("
                                              << stopCPU() << ") mSecs");
  res.freeResult();
} // findSvcFeesCurrencyInfo()

///////////////////////////////////////////////////////////
//  QueryGetSvcFeesCurrencyHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetSvcFeesCurrencyHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetSvcFeesCurrencyHistorical"));
std::string QueryGetSvcFeesCurrencyHistorical::_baseSQL;
bool QueryGetSvcFeesCurrencyHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSvcFeesCurrencyHistorical> g_GetSvcFeesCurrencyHistorical;

const char*
QueryGetSvcFeesCurrencyHistorical::getQueryName() const
{
  return "GETSVCFEESCURRENCYHISTORICAL";
}

void
QueryGetSvcFeesCurrencyHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSvcFeesCurrencyHistoricalSQLStatement<QueryGetSvcFeesCurrencyHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSVCFEESCURRENCYHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetSvcFeesCurrencyHistorical::findSvcFeesCurrencyInfo(
    std::vector<tse::SvcFeesCurrencyInfo*>& currency,
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
  while ((row = res.nextRow()))
  {
    currency.push_back(QueryGetSvcFeesCurrencyHistoricalSQLStatement<
        QueryGetSvcFeesCurrencyHistorical>::mapRowToSvcFeesCurrencyInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETSVCFEESCURRENCYHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findSvcFeesCurrencyInfo()
}
