//----------------------------------------------------------------------------
//  File:           QueryGetFreeBaggage.cpp
//  Description:    QueryGetFreeBaggage
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
#include "DBAccess/Queries/QueryGetFreeBaggage.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFreeBaggageSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetFreeBaggage::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetFreeBaggage"));
std::string QueryGetFreeBaggage::_baseSQL;
bool QueryGetFreeBaggage::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFreeBaggage> g_GetFreeBaggage;

const char*
QueryGetFreeBaggage::getQueryName() const
{
  return "GETFREEBAGGAGE";
}

void
QueryGetFreeBaggage::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFreeBaggageSQLStatement<QueryGetFreeBaggage> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFREEBAGGAGE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetFreeBaggage::findFreeBaggage(std::vector<tse::FreeBaggageInfo*>& lstBag,
                                     CarrierCode carrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(carrier, 1);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    FreeBaggageInfo* bag =
        QueryGetFreeBaggageSQLStatement<QueryGetFreeBaggage>::mapRowToFreeBaggage(row);
    if (bag)
      lstBag.push_back(bag);
  }
  LOG4CXX_INFO(_logger,
               "GETFREEBAGGAGE: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                            << stopCPU() << ") mSecs");
  res.freeResult();
} // findFreeBaggage()

///////////////////////////////////////////////////////////
//  QueryGetFreeBaggageHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetFreeBaggageHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetFreeBaggageHistorical"));
std::string QueryGetFreeBaggageHistorical::_baseSQL;
bool QueryGetFreeBaggageHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFreeBaggageHistorical> g_GetFreeBaggageHistorical;

const char*
QueryGetFreeBaggageHistorical::getQueryName() const
{
  return "GETFREEBAGGAGEHISTORICAL";
}

void
QueryGetFreeBaggageHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFreeBaggageHistoricalSQLStatement<QueryGetFreeBaggageHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFREEBAGGAGEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetFreeBaggageHistorical::findFreeBaggage(std::vector<tse::FreeBaggageInfo*>& lstBag,
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
    FreeBaggageInfo* bag = QueryGetFreeBaggageHistoricalSQLStatement<
        QueryGetFreeBaggageHistorical>::mapRowToFreeBaggage(row);
    if (bag)
      lstBag.push_back(bag);
  }
  LOG4CXX_INFO(_logger,
               "GETFREEBAGGAGEHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ")");
  res.freeResult();
} // findFreeBaggage()
}
