//----------------------------------------------------------------------------
//  File:           QueryGetSvcFeesAccountCode.cpp
//  Description:    QuerySvcFeesAccountCode
//  Created:        3/10/2009
// Authors:
//
//  Updates:
//
//  2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetSvcFeesAccountCode.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetSvcFeesAccountCodeSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetSvcFeesAccountCode::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetSvcFeesAccountCode"));
std::string QueryGetSvcFeesAccountCode::_baseSQL;
bool QueryGetSvcFeesAccountCode::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSvcFeesAccountCode> g_GetSvcFeesAccountCode;

const char*
QueryGetSvcFeesAccountCode::getQueryName() const
{
  return "GETSVCFEESACCOUNTCODE";
}

void
QueryGetSvcFeesAccountCode::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSvcFeesAccountCodeSQLStatement<QueryGetSvcFeesAccountCode> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSVCFEESACCOUNTCODE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetSvcFeesAccountCode::findSvcFeesAccCodeInfo(std::vector<tse::SvcFeesAccCodeInfo*>& accCode,
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
    accCode.push_back(QueryGetSvcFeesAccountCodeSQLStatement<
        QueryGetSvcFeesAccountCode>::mapRowToSvcFeesAccountCodeInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETSVCFEESACCOUNTCODE: NumRows " << res.numRows() << " Time = " << stopTimer()
                                                 << " (" << stopCPU() << ") mSecs");
  res.freeResult();
} // findSvcFeesAccCodeInfo()

///////////////////////////////////////////////////////////
//  QueryGetSvcFeesAccountCodeHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetSvcFeesAccountCodeHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetSvcFeesAccountCodeHistorical"));
std::string QueryGetSvcFeesAccountCodeHistorical::_baseSQL;
bool QueryGetSvcFeesAccountCodeHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSvcFeesAccountCodeHistorical> g_GetSvcFeesAccountCodeHistorical;

const char*
QueryGetSvcFeesAccountCodeHistorical::getQueryName() const
{
  return "GETSVCFEESACCOUNTCODEHISTORICAL";
}

void
QueryGetSvcFeesAccountCodeHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSvcFeesAccountCodeHistoricalSQLStatement<QueryGetSvcFeesAccountCodeHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSVCFEESACCOUNTCODEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetSvcFeesAccountCodeHistorical::findSvcFeesAccCodeInfo(
    std::vector<tse::SvcFeesAccCodeInfo*>& accCode,
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
    accCode.push_back(QueryGetSvcFeesAccountCodeHistoricalSQLStatement<
        QueryGetSvcFeesAccountCodeHistorical>::mapRowToSvcFeesAccountCodeInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETSVCFEESACCOUNTCODEHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findSvcFeesAccountCodeInfo()
}
