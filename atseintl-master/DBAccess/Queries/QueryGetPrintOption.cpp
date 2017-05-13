//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetPrintOption.h"

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetPrintOptionSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetPrintOption::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetPrintOption"));
std::string QueryGetPrintOption::_baseSQL;
bool QueryGetPrintOption::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetPrintOption> g_GetPrintOption;

const char*
QueryGetPrintOption::getQueryName() const
{
  return "GETPRINTOPTION";
}

void
QueryGetPrintOption::initialize()
{
  if (!_isInitialized)
  {
    QueryGetPrintOptionSQLStatement<QueryGetPrintOption> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETPRINTOPTION");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetPrintOption::findPrintOption(std::vector<PrintOption*>& printOptions,
                                     const VendorCode& vendor,
                                     const std::string& fareSource)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(vendor, 1);
  substParm(fareSource, 2);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    printOptions.push_back(
        QueryGetPrintOptionSQLStatement<QueryGetPrintOption>::mapRowToPrintOption(row));
  }
  LOG4CXX_INFO(_logger,
               "GETPRINTOPTION: NumRows " << res.numRows() << " Time = " << stopTimer() << " ("
                                          << stopCPU() << ") mSecs");
  res.freeResult();
}

///////////////////////////////////////////////////////////
//  QueryGetPrintOptionHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetPrintOptionHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetPrintOptionHistorical"));
std::string QueryGetPrintOptionHistorical::_baseSQL;
bool QueryGetPrintOptionHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetPrintOptionHistorical> g_GetPrintOptionHistorical;

const char*
QueryGetPrintOptionHistorical::getQueryName() const
{
  return "GETPRINTOPTIONHISTORICAL";
}

void
QueryGetPrintOptionHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetPrintOptionHistoricalSQLStatement<QueryGetPrintOptionHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETPRINTOPTIONHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetPrintOptionHistorical::findPrintOption(std::vector<PrintOption*>& printOptions,
                                               const VendorCode& vendor,
                                               const std::string& fareSource,
                                               const DateTime& startDate,
                                               const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(vendor, 1);
  substParm(fareSource, 2);
  substParm(3, startDate);
  substParm(4, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    printOptions.push_back(QueryGetPrintOptionHistoricalSQLStatement<
        QueryGetPrintOptionHistorical>::mapRowToPrintOption(row));
  }
  LOG4CXX_INFO(_logger,
               "GETPRINTOPTIONHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ")");
  res.freeResult();
}
} // tse
