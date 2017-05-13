//----------------------------------------------------------------------------
//  File:           QueryGetDayTimeApplications.cpp
//  Description:    QueryGetDayTimeApplications
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
#include "DBAccess/Queries/QueryGetDayTimeApplications.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetDayTimeApplicationsSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetDayTimeApplications::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetDayTimeApplications"));
std::string QueryGetDayTimeApplications::_baseSQL;
bool QueryGetDayTimeApplications::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetDayTimeApplications> g_GetDayTimeApplications;

const char*
QueryGetDayTimeApplications::getQueryName() const
{
  return "GETDAYTIMEAPPLICATIONS";
}

void
QueryGetDayTimeApplications::initialize()
{
  if (!_isInitialized)
  {
    QueryGetDayTimeApplicationsSQLStatement<QueryGetDayTimeApplications> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETDAYTIMEAPPLICATIONS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetDayTimeApplications::findDayTimeAppInfo(std::vector<tse::DayTimeAppInfo*>& dtApps,
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
    dtApps.push_back(QueryGetDayTimeApplicationsSQLStatement<
        QueryGetDayTimeApplications>::mapRowToDayTimeAppInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETDAYTIMEAPPLICATIONS: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                    << " (" << stopCPU() << ")");
  res.freeResult();
} // findDayTimeAppInfo()

///////////////////////////////////////////////////////////
//  QueryGetDayTimeApplicationsHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetDayTimeApplicationsHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetDayTimeApplicationsHistorical"));
std::string QueryGetDayTimeApplicationsHistorical::_baseSQL;
bool QueryGetDayTimeApplicationsHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetDayTimeApplicationsHistorical> g_GetDayTimeApplicationsHistorical;

const char*
QueryGetDayTimeApplicationsHistorical::getQueryName() const
{
  return "GETDAYTIMEAPPLICATIONSHISTORICAL";
}

void
QueryGetDayTimeApplicationsHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetDayTimeApplicationsHistoricalSQLStatement<QueryGetDayTimeApplicationsHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETDAYTIMEAPPLICATIONSHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetDayTimeApplicationsHistorical::findDayTimeAppInfo(std::vector<tse::DayTimeAppInfo*>& dtApps,
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
    dtApps.push_back(QueryGetDayTimeApplicationsHistoricalSQLStatement<
        QueryGetDayTimeApplicationsHistorical>::mapRowToDayTimeAppInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETDAYTIMEAPPLICATIONSHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findDayTimeAppInfo()
}
