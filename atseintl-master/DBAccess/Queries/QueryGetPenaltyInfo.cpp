//----------------------------------------------------------------------------
//  File:           QueryGetPenaltyInfo.cpp
//  Description:    QueryGetPenaltyInfo
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
#include "DBAccess/Queries/QueryGetPenaltyInfo.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetPenaltyInfoSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetPenaltyInfo::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetPenaltyInfo"));
std::string QueryGetPenaltyInfo::_baseSQL;
bool QueryGetPenaltyInfo::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetPenaltyInfo> g_GetPenaltyInfo;

const char*
QueryGetPenaltyInfo::getQueryName() const
{
  return "GETPENALTYINFO";
}

void
QueryGetPenaltyInfo::initialize()
{
  if (!_isInitialized)
  {
    QueryGetPenaltyInfoSQLStatement<QueryGetPenaltyInfo> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETPENALTYINFO");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetPenaltyInfo::findPenaltyInfo(std::vector<tse::PenaltyInfo*>& lstPI,
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
    lstPI.push_back(QueryGetPenaltyInfoSQLStatement<QueryGetPenaltyInfo>::mapRowToPenaltyInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETPENALTYINFO: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                            << stopCPU() << ") mSecs");
  res.freeResult();
} // findPenaltyInfo()

///////////////////////////////////////////////////////////
//  QueryGetPenaltyInfoHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetPenaltyInfoHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetPenaltyInfoHistorical"));
std::string QueryGetPenaltyInfoHistorical::_baseSQL;
bool QueryGetPenaltyInfoHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetPenaltyInfoHistorical> g_GetPenaltyInfoHistorical;

const char*
QueryGetPenaltyInfoHistorical::getQueryName() const
{
  return "GETPENALTYINFOHISTORICAL";
}

void
QueryGetPenaltyInfoHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetPenaltyInfoHistoricalSQLStatement<QueryGetPenaltyInfoHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETPENALTYINFOHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetPenaltyInfoHistorical::findPenaltyInfo(std::vector<tse::PenaltyInfo*>& lstPI,
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
    lstPI.push_back(QueryGetPenaltyInfoHistoricalSQLStatement<
        QueryGetPenaltyInfoHistorical>::mapRowToPenaltyInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETPENALTYINFOHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ")");
  res.freeResult();
} // findPenaltyInfo()
}
