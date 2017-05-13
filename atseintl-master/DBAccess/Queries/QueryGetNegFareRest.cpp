//----------------------------------------------------------------------------
//  File:           QueryGetNegFareRest.cpp
//  Description:    QueryGetNegFareRest
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
#include "DBAccess/Queries/QueryGetNegFareRest.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetNegFareRestSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetNegFareRest::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetNegFareRest"));
std::string QueryGetNegFareRest::_baseSQL;
bool QueryGetNegFareRest::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetNegFareRest> g_GetNegFareRest;

const char*
QueryGetNegFareRest::getQueryName() const
{
  return "GETNEGFAREREST";
}

void
QueryGetNegFareRest::initialize()
{
  if (!_isInitialized)
  {
    QueryGetNegFareRestSQLStatement<QueryGetNegFareRest> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETNEGFAREREST");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetNegFareRest::findNegFareRest(std::vector<tse::NegFareRest*>& lstNFR,
                                     VendorCode& vendor,
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
    lstNFR.push_back(
        QueryGetNegFareRestSQLStatement<QueryGetNegFareRest>::mapRowToNegFareRest(row));
  }
  LOG4CXX_INFO(_logger,
               "GETNEGFAREREST: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                            << stopCPU() << ")");
  res.freeResult();
} // findNegFareRest()

///////////////////////////////////////////////////////////
//  QueryGetNegFareRestHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetNegFareRestHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetNegFareRestHistorical"));
std::string QueryGetNegFareRestHistorical::_baseSQL;
bool QueryGetNegFareRestHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetNegFareRestHistorical> g_GetNegFareRestHistorical;

const char*
QueryGetNegFareRestHistorical::getQueryName() const
{
  return "GETNEGFARERESTHISTORICAL";
}

void
QueryGetNegFareRestHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetNegFareRestHistoricalSQLStatement<QueryGetNegFareRestHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETNEGFARERESTHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetNegFareRestHistorical::findNegFareRest(std::vector<tse::NegFareRest*>& lstNFR,
                                               VendorCode& vendor,
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
  substParm(vendor, 5);
  substParm(itemStr, 6);
  substParm(7, startDate);
  substParm(8, endDate);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstNFR.push_back(QueryGetNegFareRestHistoricalSQLStatement<
        QueryGetNegFareRestHistorical>::mapRowToNegFareRest(row));
  }
  LOG4CXX_INFO(_logger,
               "GETNEGFARERESTHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ")");
  res.freeResult();
} // findNegFareRest()
}
