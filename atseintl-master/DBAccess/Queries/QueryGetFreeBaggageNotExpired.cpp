//----------------------------------------------------------------------------
//  File:           QueryGetFreeBaggageNotExpired.cpp
//  Description:    QueryGetFreeBaggageNotExpired
//  Created:        06/27/2007
// Authors:         Slawomir Machowicz
//
//  Updates:
//
// � 2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetFreeBaggageNotExpired.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFreeBaggageNotExpiredSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetFreeBaggageNotExpired::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetFreeBaggageNotExpired"));
std::string QueryGetFreeBaggageNotExpired::_baseSQL;
bool QueryGetFreeBaggageNotExpired::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFreeBaggageNotExpired> g_GetFreeBaggageNotExpired;

const char*
QueryGetFreeBaggageNotExpired::getQueryName() const
{
  return "GETFREEBAGGAGENOTEXPIRED";
}

void
QueryGetFreeBaggageNotExpired::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFreeBaggageNotExpiredSQLStatement<QueryGetFreeBaggageNotExpired> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFREEBAGGAGENOTEXPIRED");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetFreeBaggageNotExpired::findFreeBaggage(std::vector<tse::FreeBaggageInfo*>& lstBag,
                                               CarrierCode carrier,
                                               DateTime& reqDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  substParm(carrier, 1);

  // std::string date ;
  // convertDate(date,reqDate);
  // substParm(date,2);
  substParm(2, reqDate);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    FreeBaggageInfo* bag = QueryGetFreeBaggageNotExpiredSQLStatement<
        QueryGetFreeBaggageNotExpired>::mapRowToFreeBaggage(row);
    if (bag)
      lstBag.push_back(bag);
  }
  LOG4CXX_INFO(_logger,
               "GETFREEBAGGAGENOTEXPIRED: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ") mSecs");
  res.freeResult();
} // findFreeBaggage()

///////////////////////////////////////////////////////////
//  QueryGetFreeBaggageNotExpiredHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetFreeBaggageNotExpiredHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.QueryGetFreeBaggageNotExpiredHistorical"));
std::string QueryGetFreeBaggageNotExpiredHistorical::_baseSQL;
bool QueryGetFreeBaggageNotExpiredHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFreeBaggageNotExpiredHistorical>
g_GetFreeBaggageNotExpiredHistorical;

const char*
QueryGetFreeBaggageNotExpiredHistorical::getQueryName() const
{
  return "GETFREEBAGGAGENOTEXPIREDHISTORICAL";
}

void
QueryGetFreeBaggageNotExpiredHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFreeBaggageNotExpiredHistoricalSQLStatement<QueryGetFreeBaggageNotExpiredHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFREEBAGGAGENOTEXPIREDHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetFreeBaggageNotExpiredHistorical::findFreeBaggage(std::vector<tse::FreeBaggageInfo*>& lstBag,
                                                         CarrierCode carrier,
                                                         const DateTime& startDate,
                                                         const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, carrier);
  substParm(2, startDate);
  substParm(3, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    FreeBaggageInfo* bag = QueryGetFreeBaggageNotExpiredHistoricalSQLStatement<
        QueryGetFreeBaggageNotExpiredHistorical>::mapRowToFreeBaggage(row);
    if (bag)
      lstBag.push_back(bag);
  }
  LOG4CXX_INFO(_logger,
               "GETFREEBAGGAGENOTEXPIREDHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findFreeBaggage()
}
