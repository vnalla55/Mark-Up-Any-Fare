//----------------------------------------------------------------------------
//  File:           QueryGetTable988.cpp
//  Description:    QueryGetTable988
//  Created:        4/3/2007
// Authors:         Grzegorz Cholewiak
//
//  Updates:
//
// � 2007, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetTable988.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetTable988SQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetTable988::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTable988"));
std::string QueryGetTable988::_baseSQL;
bool QueryGetTable988::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTable988> g_GetTable988;

const char*
QueryGetTable988::getQueryName() const
{
  return "GETTABLE988";
}

void
QueryGetTable988::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTable988SQLStatement<QueryGetTable988> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTABLE988");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTable988::findReissue(std::vector<tse::ReissueSequence*>& table988s,
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
    table988s.push_back(
        QueryGetTable988SQLStatement<QueryGetTable988>::mapRowToReissueSequence(row));
  }
  LOG4CXX_INFO(_logger,
               "GETTABLE988: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                         << stopCPU() << ")");
  res.freeResult();
} // findReissue()

///////////////////////////////////////////////////////////
//  QueryGetTable988Historical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetTable988Historical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetTable988Historical"));
std::string QueryGetTable988Historical::_baseSQL;
bool QueryGetTable988Historical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTable988Historical> g_GetTable988Historical;

const char*
QueryGetTable988Historical::getQueryName() const
{
  return "GETTABLE988HISTORICAL";
}

void
QueryGetTable988Historical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTable988HistoricalSQLStatement<QueryGetTable988Historical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTABLE988HISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTable988Historical::findReissue(std::vector<tse::ReissueSequence*>& table988s,
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
    table988s.push_back(
        QueryGetTable988HistoricalSQLStatement<QueryGetTable988Historical>::mapRowToReissueSequence(
            row));
  }
  LOG4CXX_INFO(_logger,
               "GETTABLE988HISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                   << " (" << stopCPU() << ")");
  res.freeResult();
} // findReissue()
}
