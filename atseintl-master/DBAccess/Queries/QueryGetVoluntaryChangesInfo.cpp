//----------------------------------------------------------------------------
//  File:           QueryGetVoluntaryChangesInfo.cpp
//  Description:    QueryGetVoluntaryChangesInfo
//  Created:        3/29/2007
// Authors:         Artur Krezel
//
//  Updates:
//
// � 2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetVoluntaryChangesInfo.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetVoluntaryChangesInfoSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetVoluntaryChangesInfo::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetVoluntaryChangesInfo"));
std::string QueryGetVoluntaryChangesInfo::_baseSQL;
bool QueryGetVoluntaryChangesInfo::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetVoluntaryChangesInfo> g_GetVoluntaryChangesInfo;

const char*
QueryGetVoluntaryChangesInfo::getQueryName() const
{
  return "GETVOLUNTARYCHANGESINFO";
}

void
QueryGetVoluntaryChangesInfo::initialize()
{
  if (!_isInitialized)
  {
    QueryGetVoluntaryChangesInfoSQLStatement<QueryGetVoluntaryChangesInfo> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETVOLUNTARYCHANGESINFO");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetVoluntaryChangesInfo::findVoluntaryChangesInfo(
    std::vector<tse::VoluntaryChangesInfo*>& lstVCI, VendorCode& vendor, int itemNo)
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
    lstVCI.push_back(QueryGetVoluntaryChangesInfoSQLStatement<
        QueryGetVoluntaryChangesInfo>::mapRowToVoluntaryChangesInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETVOLUNTARYCHANGES: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                 << " (" << stopCPU() << ")");
  res.freeResult();
} // findVoluntaryChangesInfo()

///////////////////////////////////////////////////////////
//  QueryGetVoluntaryChangesInfoHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetVoluntaryChangesInfoHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.QueryGetVoluntaryChangesInfoHistorical"));
std::string QueryGetVoluntaryChangesInfoHistorical::_baseSQL;
bool QueryGetVoluntaryChangesInfoHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetVoluntaryChangesInfoHistorical>
g_GetVoluntaryChangesInfoHistorical;

const char*
QueryGetVoluntaryChangesInfoHistorical::getQueryName() const
{
  return "GETVOLUNTARYCHANGESINFOHISTORICAL";
}

void
QueryGetVoluntaryChangesInfoHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetVoluntaryChangesInfoHistoricalSQLStatement<QueryGetVoluntaryChangesInfoHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETVOLUNTARYCHANGESINFOHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetVoluntaryChangesInfoHistorical::findVoluntaryChangesInfo(
    std::vector<tse::VoluntaryChangesInfo*>& lstVCI,
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
    lstVCI.push_back(QueryGetVoluntaryChangesInfoHistoricalSQLStatement<
        QueryGetVoluntaryChangesInfoHistorical>::mapRowToVoluntaryChangesInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETVOLUNTARYCHANGESINFOHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findVoluntaryChangesInfo()
}
