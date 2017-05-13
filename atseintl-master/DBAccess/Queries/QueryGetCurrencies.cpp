//----------------------------------------------------------------------------
//  File:           QueryGetCurrencies.cpp
//  Description:    QueryGetCurrencies
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

#include "DBAccess/Queries/QueryGetCurrencies.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCurrenciesSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetCurrencies::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetCurrencies"));
std::string QueryGetCurrencies::_baseSQL;
bool QueryGetCurrencies::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCurrencies> g_GetCurrencies;

const char*
QueryGetCurrencies::getQueryName() const
{
  return "GETCURRENCIES";
}

void
QueryGetCurrencies::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCurrenciesSQLStatement<QueryGetCurrencies> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCURRENCIES");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetCurrencies::findCurrency(std::vector<tse::Currency*>& lstCurr, const CurrencyCode& cur)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, cur);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::Currency* curr = nullptr;
  tse::Currency* currPrev = nullptr;
  while ((row = res.nextRow()) != nullptr)
  {
    curr = QueryGetCurrenciesSQLStatement<QueryGetCurrencies>::mapRowToCurrency(row, currPrev);
    if (curr != currPrev)
      lstCurr.push_back(curr);

    currPrev = curr;
  }
  LOG4CXX_INFO(_logger,
               "GETCURRENCIES: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                           << stopCPU() << ")");
  res.freeResult();
} // findCurrency()
///////////////////////////////////////////////////////////
//
//  QueryGetCurrenciesHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetCurrenciesHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetCurrenciesHistorical"));
std::string QueryGetCurrenciesHistorical::_baseSQL;
bool QueryGetCurrenciesHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCurrenciesHistorical> g_GetCurrenciesHistorical;

const char*
QueryGetCurrenciesHistorical::getQueryName() const
{
  return "GETCURRENCIESHISTORICAL";
}

void
QueryGetCurrenciesHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCurrenciesHistoricalSQLStatement<QueryGetCurrenciesHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCURRENCIESHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetCurrenciesHistorical::findCurrency(std::vector<tse::Currency*>& lstCurr,
                                           const CurrencyCode& cur)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, cur);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::Currency* curr = nullptr;
  tse::Currency* currPrev = nullptr;
  while ((row = res.nextRow()) != nullptr)
  {
    curr = QueryGetCurrenciesHistoricalSQLStatement<QueryGetCurrenciesHistorical>::mapRowToCurrency(
        row, currPrev);
    if (curr != currPrev)
      lstCurr.push_back(curr);

    currPrev = curr;
  }
  LOG4CXX_INFO(_logger,
               "GETCURRENCIESHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                     << " (" << stopCPU() << ")");
  res.freeResult();
} // findCurrency()

///////////////////////////////////////////////////////////
//
//  QueryGetAllCurrencies
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllCurrencies::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllCurrencies"));
std::string QueryGetAllCurrencies::_baseSQL;
bool QueryGetAllCurrencies::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllCurrencies> g_GetAllCurrencies;

const char*
QueryGetAllCurrencies::getQueryName() const
{
  return "GETALLCURRENCIES";
}

void
QueryGetAllCurrencies::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllCurrenciesSQLStatement<QueryGetAllCurrencies> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLCURRENCIES");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllCurrencies::findAllCurrency(std::vector<tse::Currency*>& lstCurr)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::Currency* curr = nullptr;
  tse::Currency* currPrev = nullptr;
  while ((row = res.nextRow()) != nullptr)
  {
    curr =
        QueryGetAllCurrenciesSQLStatement<QueryGetAllCurrencies>::mapRowToCurrency(row, currPrev);
    if (curr != currPrev)
      lstCurr.push_back(curr);

    currPrev = curr;
  }
  LOG4CXX_INFO(_logger,
               "GETALLCURRENCIES: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                              << stopCPU() << ")");
  res.freeResult();
} // findAllCurrency()
///////////////////////////////////////////////////////////
//
//  QueryGetAllCurrenciesHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllCurrenciesHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllCurrenciesHistorical"));
std::string QueryGetAllCurrenciesHistorical::_baseSQL;
bool QueryGetAllCurrenciesHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllCurrenciesHistorical> g_GetAllCurrenciesHistorical;

const char*
QueryGetAllCurrenciesHistorical::getQueryName() const
{
  return "GETALLCURRENCIESHISTORICAL";
}

void
QueryGetAllCurrenciesHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllCurrenciesHistoricalSQLStatement<QueryGetAllCurrenciesHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLCURRENCIESHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllCurrenciesHistorical::findAllCurrency(std::vector<tse::Currency*>& lstCurr)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::Currency* curr = nullptr;
  tse::Currency* currPrev = nullptr;
  while ((row = res.nextRow()) != nullptr)
  {
    curr = QueryGetAllCurrenciesHistoricalSQLStatement<
        QueryGetAllCurrenciesHistorical>::mapRowToCurrency(row, currPrev);
    if (curr != currPrev)
      lstCurr.push_back(curr);

    currPrev = curr;
  }
  LOG4CXX_INFO(_logger,
               "GETALLCURRENCIESHISTORICAL: NumRows = " << res.numRows() << " Time = "
                                                        << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllCurrency()
}
