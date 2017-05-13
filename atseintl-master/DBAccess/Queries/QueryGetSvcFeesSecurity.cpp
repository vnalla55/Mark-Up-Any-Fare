//----------------------------------------------------------------------------
//  File:           QueryGetSvcFeesSecurity.cpp
//  Description:    QuerySvcFeesSecurity
//  Created:        3/13/2009
// Authors:
//
//  Updates:
//
//  2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetSvcFeesSecurity.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetSvcFeesSecuritySQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetSvcFeesSecurity::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetSvcFeesSecurity"));
std::string QueryGetSvcFeesSecurity::_baseSQL;
bool QueryGetSvcFeesSecurity::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSvcFeesSecurity> g_GetSvcFeesSecurity;

const char*
QueryGetSvcFeesSecurity::getQueryName() const
{
  return "GETSVCFEESSECURITY";
}

void
QueryGetSvcFeesSecurity::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSvcFeesSecuritySQLStatement<QueryGetSvcFeesSecurity> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSVCFEESSECURITY");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetSvcFeesSecurity::findSvcFeesSecurityInfo(std::vector<tse::SvcFeesSecurityInfo*>& sec,
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
    sec.push_back(
        QueryGetSvcFeesSecuritySQLStatement<QueryGetSvcFeesSecurity>::mapRowToSvcFeesSecurityInfo(
            row));
  }
  LOG4CXX_INFO(_logger,
               "GETSVCFEESSECURITY: NumRows " << res.numRows() << " Time = " << stopTimer() << " ("
                                              << stopCPU() << ") mSecs");
  res.freeResult();
} // findSvcFeesSecurityInfo()

///////////////////////////////////////////////////////////
//  QueryGetSvcFeesSecurityHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetSvcFeesSecurityHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetSvcFeesSecurityHistorical"));
std::string QueryGetSvcFeesSecurityHistorical::_baseSQL;
bool QueryGetSvcFeesSecurityHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSvcFeesSecurityHistorical> g_GetSvcFeesSecurityHistorical;

const char*
QueryGetSvcFeesSecurityHistorical::getQueryName() const
{
  return "GETSVCFEESSECURITYHISTORICAL";
}

void
QueryGetSvcFeesSecurityHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSvcFeesSecurityHistoricalSQLStatement<QueryGetSvcFeesSecurityHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSVCFEESSECURITYHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetSvcFeesSecurityHistorical::findSvcFeesSecurityInfo(
    std::vector<tse::SvcFeesSecurityInfo*>& sec,
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
    sec.push_back(QueryGetSvcFeesSecurityHistoricalSQLStatement<
        QueryGetSvcFeesSecurityHistorical>::mapRowToSvcFeesSecurityInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETSVCFEESSECURITYHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findSvcFeesSecurityInfo()
}
