//----------------------------------------------------------------------------
//  File:           QueryGetCarrierApplicationInfo.cpp
//  Description:    QueryGetCarrierApplicationInfo
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
#include "DBAccess/Queries/QueryGetCarrierApplicationInfo.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCarrierApplicationInfoSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetCarrierApplicationInfo::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetCarrierApplicationInfo"));
std::string QueryGetCarrierApplicationInfo::_baseSQL;
bool QueryGetCarrierApplicationInfo::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCarrierApplicationInfo> g_GetCarrierApplicationInfo;

const char*
QueryGetCarrierApplicationInfo::getQueryName() const
{
  return "GETCARRIERAPPLICATIONINFO";
}

void
QueryGetCarrierApplicationInfo::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCarrierApplicationInfoSQLStatement<QueryGetCarrierApplicationInfo> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCARRIERAPPLICATIONINFO");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetCarrierApplicationInfo::findCarrierApplicationInfo(
    std::vector<tse::CarrierApplicationInfo*>& lstCAI, VendorCode& vendor, int itemNo)
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
    lstCAI.push_back(QueryGetCarrierApplicationInfoSQLStatement<
        QueryGetCarrierApplicationInfo>::mapRowToCarrierApplicationInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETCARRIERAPPLICATIONINFO: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                       << " (" << stopCPU() << ")");
  res.freeResult();
} // findCarrierApplicationInfo()

///////////////////////////////////////////////////////////
//  QueryGetCarrierApplicationInfoHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetCarrierApplicationInfoHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.QueryGetCarrierApplicationInfoHistorical"));
std::string QueryGetCarrierApplicationInfoHistorical::_baseSQL;
bool QueryGetCarrierApplicationInfoHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCarrierApplicationInfoHistorical>
g_GetCarrierApplicationInfoHistorical;

const char*
QueryGetCarrierApplicationInfoHistorical::getQueryName() const
{
  return "GETCARRIERAPPLICATIONINFOHISTORICAL";
}

void
QueryGetCarrierApplicationInfoHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCarrierApplicationInfoHistoricalSQLStatement<QueryGetCarrierApplicationInfoHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCARRIERAPPLICATIONINFOHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetCarrierApplicationInfoHistorical::findCarrierApplicationInfo(
    std::vector<tse::CarrierApplicationInfo*>& lstCAI,
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
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstCAI.push_back(QueryGetCarrierApplicationInfoHistoricalSQLStatement<
        QueryGetCarrierApplicationInfoHistorical>::mapRowToCarrierApplicationInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETCARRIERAPPLICATIONINFOHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findCarrierApplicationInfo()
}
