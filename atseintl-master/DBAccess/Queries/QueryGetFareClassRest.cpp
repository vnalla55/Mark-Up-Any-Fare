//----------------------------------------------------------------------------
//  File:           QueryGetFareClassRest.cpp
//  Description:    QueryGetFareClassRest
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
#include "DBAccess/Queries/QueryGetFareClassRest.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareClassRestSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetFareClassRest::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetFareClassRest"));
std::string QueryGetFareClassRest::_baseSQL;
bool QueryGetFareClassRest::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFareClassRest> g_GetFareClassRest;

const char*
QueryGetFareClassRest::getQueryName() const
{
  return "GETFARECLASSREST";
}

void
QueryGetFareClassRest::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareClassRestSQLStatement<QueryGetFareClassRest> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFARECLASSREST");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetFareClassRest::findFareClassRestRule(std::vector<tse::FareClassRestRule*>& fareClassRests,
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
    fareClassRests.push_back(
        QueryGetFareClassRestSQLStatement<QueryGetFareClassRest>::mapRowToFareClassRestRule(row));
  }
  LOG4CXX_INFO(_logger,
               "GETFARECLASSREST: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                              << stopCPU() << ")");
  res.freeResult();
} // findFareClassRestRule()

///////////////////////////////////////////////////////////
//  QueryGetFareClassRestHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetFareClassRestHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetFareClassRestHistorical"));
std::string QueryGetFareClassRestHistorical::_baseSQL;
bool QueryGetFareClassRestHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFareClassRestHistorical> g_GetFareClassRestHistorical;

const char*
QueryGetFareClassRestHistorical::getQueryName() const
{
  return "GETFARECLASSRESTHISTORICAL";
}

void
QueryGetFareClassRestHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareClassRestHistoricalSQLStatement<QueryGetFareClassRestHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFARECLASSRESTHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetFareClassRestHistorical::findFareClassRestRule(
    std::vector<tse::FareClassRestRule*>& fareClassRests,
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
    fareClassRests.push_back(QueryGetFareClassRestHistoricalSQLStatement<
        QueryGetFareClassRestHistorical>::mapRowToFareClassRestRule(row));
  }
  LOG4CXX_INFO(_logger,
               "GETFARECLASSRESTHISTORICAL: NumRows = " << res.numRows() << " Time = "
                                                        << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findFareClassRestRule()
}
