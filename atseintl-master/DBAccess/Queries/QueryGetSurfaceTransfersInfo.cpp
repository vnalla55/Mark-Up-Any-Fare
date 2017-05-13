//----------------------------------------------------------------------------
//  File:           QueryGetSurfaceTransfersInfo.cpp
//  Description:    QueryGetSurfaceTransfersInfo
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
#include "DBAccess/Queries/QueryGetSurfaceTransfersInfo.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetSurfaceTransfersInfoSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetSurfaceTransfersInfo::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTransfersInfo1"));
std::string QueryGetSurfaceTransfersInfo::_baseSQL;
bool QueryGetSurfaceTransfersInfo::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSurfaceTransfersInfo> g_GetSurfaceTransfersInfo;

const char*
QueryGetSurfaceTransfersInfo::getQueryName() const
{
  return "GETSURFACETRANSFERSINFO";
}

void
QueryGetSurfaceTransfersInfo::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSurfaceTransfersInfoSQLStatement<QueryGetSurfaceTransfersInfo> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSURFACETRANSFERSINFO");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetSurfaceTransfersInfo::findSurfaceTransfersInfo(
    std::vector<tse::SurfaceTransfersInfo*>& lstTI, VendorCode& vendor, int itemNo)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char strItemNo[20];
  sprintf(strItemNo, "%d", itemNo);

  substParm(vendor, 1);
  substParm(strItemNo, 2);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstTI.push_back(QueryGetSurfaceTransfersInfoSQLStatement<
        QueryGetSurfaceTransfersInfo>::mapRowToSurfaceTransfersInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETSURFACETRANSFERSINFO: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                     << " (" << stopCPU() << ")");
  res.freeResult();
} // findSurfaceTransfersInfo()

///////////////////////////////////////////////////////////
//  QueryGetSurfaceTransfersInfoHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetSurfaceTransfersInfoHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.QueryGetSurfaceTransfersInfoHistorical"));
std::string QueryGetSurfaceTransfersInfoHistorical::_baseSQL;
bool QueryGetSurfaceTransfersInfoHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSurfaceTransfersInfoHistorical>
g_GetSurfaceTransfersInfoHistorical;

const char*
QueryGetSurfaceTransfersInfoHistorical::getQueryName() const
{
  return "GETSURFACETRANSFERSINFOHISTORICAL";
}

void
QueryGetSurfaceTransfersInfoHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSurfaceTransfersInfoHistoricalSQLStatement<QueryGetSurfaceTransfersInfoHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSURFACETRANSFERSINFOHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetSurfaceTransfersInfoHistorical::findSurfaceTransfersInfo(
    std::vector<tse::SurfaceTransfersInfo*>& lstTI,
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
    lstTI.push_back(QueryGetSurfaceTransfersInfoHistoricalSQLStatement<
        QueryGetSurfaceTransfersInfoHistorical>::mapRowToSurfaceTransfersInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETSURFACETRANSFERSINFOHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findSurfaceTransfersInfo()
}
