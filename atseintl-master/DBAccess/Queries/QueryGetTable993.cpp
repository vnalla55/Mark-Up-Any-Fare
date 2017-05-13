//----------------------------------------------------------------------------
//  File:           QueryGetTable993.cpp
//  Description:    QueryGetTable993
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
#include "DBAccess/Queries/QueryGetTable993.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetTable993SQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetTable993::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTable993"));
std::string QueryGetTable993::_baseSQL;
bool QueryGetTable993::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTable993> g_GetTable993;

const char*
QueryGetTable993::getQueryName() const
{
  return "GETTABLE993";
}

void
QueryGetTable993::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTable993SQLStatement<QueryGetTable993> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTABLE993");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTable993::findSamePoint(std::vector<const tse::SamePoint*>& table993s,
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
    table993s.push_back(QueryGetTable993SQLStatement<QueryGetTable993>::mapRowToSamePoint(row));
  }
  LOG4CXX_INFO(_logger,
               "GETTABLE993: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                         << stopCPU() << ")");
  res.freeResult();
} // findSamePoint()

///////////////////////////////////////////////////////////
//  QueryGetTable993Historical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetTable993Historical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetTable993Historical"));
std::string QueryGetTable993Historical::_baseSQL;
bool QueryGetTable993Historical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTable993Historical> g_GetTable993Historical;

const char*
QueryGetTable993Historical::getQueryName() const
{
  return "GETTABLE993HISTORICAL";
}

void
QueryGetTable993Historical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTable993HistoricalSQLStatement<QueryGetTable993Historical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTABLE993HISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTable993Historical::findSamePoint(std::vector<const tse::SamePoint*>& table993s,
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
    table993s.push_back(
        QueryGetTable993HistoricalSQLStatement<QueryGetTable993Historical>::mapRowToSamePoint(row));
  }
  LOG4CXX_INFO(_logger,
               "GETTABLE993HISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                   << " (" << stopCPU() << ")");
  res.freeResult();
} // findSamePoint()
}
