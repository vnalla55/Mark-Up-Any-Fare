//----------------------------------------------------------------------------
//  File:           QueryGetSeasonalAppl.cpp
//  Description:    QueryGetSeasonalAppl
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
#include "DBAccess/Queries/QueryGetSeasonalAppl.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetSeasonalApplSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetSeasonalAppl::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetSeasonalAppl"));
std::string QueryGetSeasonalAppl::_baseSQL;
bool QueryGetSeasonalAppl::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSeasonalAppl> g_GetSeasonalAppl;

const char*
QueryGetSeasonalAppl::getQueryName() const
{
  return "GETSEASONALAPPL";
}

void
QueryGetSeasonalAppl::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSeasonalApplSQLStatement<QueryGetSeasonalAppl> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSEASONALAPPL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetSeasonalAppl::findSeasonalAppl(std::vector<tse::SeasonalAppl*>& seasappls,
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
    seasappls.push_back(
        QueryGetSeasonalApplSQLStatement<QueryGetSeasonalAppl>::mapRowToSeasonalAppl(row));
  }
  LOG4CXX_INFO(_logger,
               "GETSEASONALAPPL: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                             << stopCPU() << ")");
  res.freeResult();
} // findSeasonalAppl()

///////////////////////////////////////////////////////////
//  QueryGetSeasonalApplHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetSeasonalApplHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetSeasonalApplHistorical"));
std::string QueryGetSeasonalApplHistorical::_baseSQL;
bool QueryGetSeasonalApplHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSeasonalApplHistorical> g_GetSeasonalApplHistorical;

const char*
QueryGetSeasonalApplHistorical::getQueryName() const
{
  return "GETSEASONALAPPLHISTORICAL";
}

void
QueryGetSeasonalApplHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSeasonalApplHistoricalSQLStatement<QueryGetSeasonalApplHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSEASONALAPPLHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetSeasonalApplHistorical::findSeasonalAppl(std::vector<tse::SeasonalAppl*>& seasappls,
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
    seasappls.push_back(QueryGetSeasonalApplHistoricalSQLStatement<
        QueryGetSeasonalApplHistorical>::mapRowToSeasonalAppl(row));
  }
  LOG4CXX_INFO(_logger,
               "GETSEASONALAPPLHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                       << " (" << stopCPU() << ")");
  res.freeResult();
} // findSeasonalAppl()
}
