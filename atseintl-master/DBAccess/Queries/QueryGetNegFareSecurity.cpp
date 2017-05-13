//----------------------------------------------------------------------------
//  File:           QueryGetNegFareSecurity.cpp
//  Description:    QueryGetNegFareSecurity
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
#include "DBAccess/Queries/QueryGetNegFareSecurity.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetNegFareSecuritySQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetNegFareSecurity::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetNegFareSecurity"));
std::string QueryGetNegFareSecurity::_baseSQL;
bool QueryGetNegFareSecurity::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetNegFareSecurity> g_GetNegFareSecurity;

const char*
QueryGetNegFareSecurity::getQueryName() const
{
  return "GETNEGFARESECURITY";
}

void
QueryGetNegFareSecurity::initialize()
{
  if (!_isInitialized)
  {
    QueryGetNegFareSecuritySQLStatement<QueryGetNegFareSecurity> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETNEGFARESECURITY");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetNegFareSecurity::findNegFareSecurity(std::vector<NegFareSecurityInfo*>& negFareSec,
                                             VendorCode& vendor,
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
    negFareSec.push_back(
        QueryGetNegFareSecuritySQLStatement<QueryGetNegFareSecurity>::mapRowToNegFareSecurityInfo(
            row));
  }
  LOG4CXX_INFO(_logger,
               "GETNEGFARESECURITY: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                << " (" << stopCPU() << ") mSecs");
  res.freeResult();
} // findNegFareSecurity()

///////////////////////////////////////////////////////////
//  QueryGetNegFareSecurityHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetNegFareSecurityHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetNegFareSecurityHistorical"));
std::string QueryGetNegFareSecurityHistorical::_baseSQL;
bool QueryGetNegFareSecurityHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetNegFareSecurityHistorical> g_GetNegFareSecurityHistorical;

const char*
QueryGetNegFareSecurityHistorical::getQueryName() const
{
  return "GETNEGFARESECURITYHISTORICAL";
}

void
QueryGetNegFareSecurityHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetNegFareSecurityHistoricalSQLStatement<QueryGetNegFareSecurityHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETNEGFARESECURITYHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetNegFareSecurityHistorical::findNegFareSecurity(
    std::vector<NegFareSecurityInfo*>& negFareSec,
    VendorCode& vendor,
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
  substParm(vendor, 5);
  substParm(itemStr, 6);
  substParm(7, startDate);
  substParm(8, endDate);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    negFareSec.push_back(QueryGetNegFareSecurityHistoricalSQLStatement<
        QueryGetNegFareSecurityHistorical>::mapRowToNegFareSecurityInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETNEGFARESECURITYHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findNegFareSecurity()
}
