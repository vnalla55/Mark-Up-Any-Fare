//----------------------------------------------------------------------------
//  File:           QueryGetVisitAnotherCountry.cpp
//  Description:    QueryGetVisitAnotherCountry
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
#include "DBAccess/Queries/QueryGetVisitAnotherCountry.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetVisitAnotherCountrySQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetVisitAnotherCountry::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetVisitAnotherCountry"));
std::string QueryGetVisitAnotherCountry::_baseSQL;
bool QueryGetVisitAnotherCountry::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetVisitAnotherCountry> g_GetVisitAnotherCountry;

const char*
QueryGetVisitAnotherCountry::getQueryName() const
{
  return "GETVISITANOTHERCOUNTRY";
}

void
QueryGetVisitAnotherCountry::initialize()
{
  if (!_isInitialized)
  {
    QueryGetVisitAnotherCountrySQLStatement<QueryGetVisitAnotherCountry> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETVISITANOTHERCOUNTRY");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetVisitAnotherCountry::findVisitAnotherCountry(std::vector<tse::VisitAnotherCountry*>& visits,
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
    visits.push_back(QueryGetVisitAnotherCountrySQLStatement<
        QueryGetVisitAnotherCountry>::mapRowToVisitAnotherCountry(row));
  }
  LOG4CXX_INFO(_logger,
               "GETVISITANOTHERCOUNTRY: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                    << " (" << stopCPU() << ")");
  res.freeResult();
} // findVisitAnotherCountry()

///////////////////////////////////////////////////////////
//  QueryGetVisitAnotherCountryHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetVisitAnotherCountryHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetVisitAnotherCountryHistorical"));
std::string QueryGetVisitAnotherCountryHistorical::_baseSQL;
bool QueryGetVisitAnotherCountryHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetVisitAnotherCountryHistorical> g_GetVisitAnotherCountryHistorical;

const char*
QueryGetVisitAnotherCountryHistorical::getQueryName() const
{
  return "GETVISITANOTHERCOUNTRYHISTORICAL";
}

void
QueryGetVisitAnotherCountryHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetVisitAnotherCountryHistoricalSQLStatement<QueryGetVisitAnotherCountryHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETVISITANOTHERCOUNTRYHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetVisitAnotherCountryHistorical::findVisitAnotherCountry(
    std::vector<tse::VisitAnotherCountry*>& visits,
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
    visits.push_back(QueryGetVisitAnotherCountryHistoricalSQLStatement<
        QueryGetVisitAnotherCountryHistorical>::mapRowToVisitAnotherCountry(row));
  }
  LOG4CXX_INFO(_logger,
               "GETVISITANOTHERCOUNTRYHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findVisitAnotherCountry()
}
