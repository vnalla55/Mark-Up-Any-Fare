//----------------------------------------------------------------------------
//  File:           QueryGetCarrierComb.cpp
//  Description:    QueryGetCarrierComb
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
#include "DBAccess/Queries/QueryGetCarrierComb.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCarrierCombSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetCarrierComb::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetCarrierComb"));
std::string QueryGetCarrierComb::_baseSQL;
bool QueryGetCarrierComb::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCarrierComb> g_GetCarrierComb;

const char*
QueryGetCarrierComb::getQueryName() const
{
  return "GETCARRIERCOMB";
}

void
QueryGetCarrierComb::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCarrierCombSQLStatement<QueryGetCarrierComb> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCARRIERCOMB");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetCarrierComb::findCarrierCombination(std::vector<CarrierCombination*>& carrierCombs,
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
    carrierCombs.push_back(
        QueryGetCarrierCombSQLStatement<QueryGetCarrierComb>::mapRowToCarrierCombination(row));
  }
  LOG4CXX_INFO(_logger,
               "GETCARRIERCOMB: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                            << stopCPU() << ")");
  res.freeResult();
} // findCarrierCombination()

///////////////////////////////////////////////////////////
//  QueryGetCarrierCombHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetCarrierCombHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetCarrierCombHistorical"));
std::string QueryGetCarrierCombHistorical::_baseSQL;
bool QueryGetCarrierCombHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCarrierCombHistorical> g_GetCarrierCombHistorical;

const char*
QueryGetCarrierCombHistorical::getQueryName() const
{
  return "GETCARRIERCOMBHISTORICAL";
}

void
QueryGetCarrierCombHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCarrierCombHistoricalSQLStatement<QueryGetCarrierCombHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCARRIERCOMBHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetCarrierCombHistorical::findCarrierCombination(
    std::vector<CarrierCombination*>& carrierCombs,
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
    carrierCombs.push_back(QueryGetCarrierCombHistoricalSQLStatement<
        QueryGetCarrierCombHistorical>::mapRowToCarrierCombination(row));
  }
  LOG4CXX_INFO(_logger,
               "GETCARRIERCOMBHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ")");
  res.freeResult();
} // findCarrierCombination()
}
