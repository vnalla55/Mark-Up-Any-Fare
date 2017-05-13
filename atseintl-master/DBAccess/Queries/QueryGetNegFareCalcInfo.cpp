//----------------------------------------------------------------------------
//  File:           QueryGetNegFareCalcInfo.cpp
//  Description:    QueryGetNegFareCalcInfo
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
#include "DBAccess/Queries/QueryGetNegFareCalcInfo.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetNegFareCalcInfoSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetNegFareCalcInfo::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetNegFareCalcInfo"));
std::string QueryGetNegFareCalcInfo::_baseSQL;
bool QueryGetNegFareCalcInfo::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetNegFareCalcInfo> g_GetNegFareCalcInfo;

const char*
QueryGetNegFareCalcInfo::getQueryName() const
{
  return "GETNEGFARECALCINFO";
}

void
QueryGetNegFareCalcInfo::initialize()
{
  if (!_isInitialized)
  {
    QueryGetNegFareCalcInfoSQLStatement<QueryGetNegFareCalcInfo> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETNEGFARECALCINFO");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetNegFareCalcInfo::findNegFareCalcInfo(std::vector<tse::NegFareCalcInfo*>& lstNFC,
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
    lstNFC.push_back(
        QueryGetNegFareCalcInfoSQLStatement<QueryGetNegFareCalcInfo>::mapRowToNegFareCalcInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETNEGFARECALCINFO: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                << " (" << stopCPU() << ") mSecs");
  res.freeResult();
} // findNegFareCalcInfo()

///////////////////////////////////////////////////////////
//  QueryGetNegFareCalcInfoHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetNegFareCalcInfoHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetNegFareCalcInfoHistorical"));
std::string QueryGetNegFareCalcInfoHistorical::_baseSQL;
bool QueryGetNegFareCalcInfoHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetNegFareCalcInfoHistorical> g_GetNegFareCalcInfoHistorical;

const char*
QueryGetNegFareCalcInfoHistorical::getQueryName() const
{
  return "GETNEGFARECALCINFOHISTORICAL";
}

void
QueryGetNegFareCalcInfoHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetNegFareCalcInfoHistoricalSQLStatement<QueryGetNegFareCalcInfoHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETNEGFARECALCINFOHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetNegFareCalcInfoHistorical::findNegFareCalcInfo(std::vector<tse::NegFareCalcInfo*>& lstNFC,
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
    lstNFC.push_back(QueryGetNegFareCalcInfoHistoricalSQLStatement<
        QueryGetNegFareCalcInfoHistorical>::mapRowToNegFareCalcInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETNEGFARECALCINFOHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findNegFareCalcInfo()
}
