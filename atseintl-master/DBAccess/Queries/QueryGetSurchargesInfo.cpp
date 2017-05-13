//----------------------------------------------------------------------------
//  File:           QueryGetSurchargesInfo.cpp
//  Description:    QueryGetSurchargesInfo
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
#include "DBAccess/Queries/QueryGetSurchargesInfo.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetSurchargesInfoSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetSurchargesInfo::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetSurchargesInfo"));
std::string QueryGetSurchargesInfo::_baseSQL;
bool QueryGetSurchargesInfo::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSurchargesInfo> g_GetSurchargesInfo;

const char*
QueryGetSurchargesInfo::getQueryName() const
{
  return "GETSURCHARGESINFO";
}

void
QueryGetSurchargesInfo::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSurchargesInfoSQLStatement<QueryGetSurchargesInfo> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSURCHARGESINFO");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetSurchargesInfo::findSurchargesInfo(std::vector<tse::SurchargesInfo*>& lstSI,
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
    lstSI.push_back(
        QueryGetSurchargesInfoSQLStatement<QueryGetSurchargesInfo>::mapRowToSurchargesInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETSURCHARGESINFO: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                               << stopCPU() << ")");
  res.freeResult();
} // findSurchargesInfo()

///////////////////////////////////////////////////////////
//  QueryGetSurchargesInfoHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetSurchargesInfoHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetSurchargesInfoHistorical"));
std::string QueryGetSurchargesInfoHistorical::_baseSQL;
bool QueryGetSurchargesInfoHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSurchargesInfoHistorical> g_GetSurchargesInfoHistorical;

const char*
QueryGetSurchargesInfoHistorical::getQueryName() const
{
  return "GETSURCHARGESINFOHISTORICAL";
}

void
QueryGetSurchargesInfoHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSurchargesInfoHistoricalSQLStatement<QueryGetSurchargesInfoHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSURCHARGESINFOHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetSurchargesInfoHistorical::findSurchargesInfo(std::vector<tse::SurchargesInfo*>& lstSI,
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
    lstSI.push_back(QueryGetSurchargesInfoHistoricalSQLStatement<
        QueryGetSurchargesInfoHistorical>::mapRowToSurchargesInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETSURCHARGESINFOHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findSurchargesInfo()
}
