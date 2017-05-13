//----------------------------------------------------------------------------
//  File:           QueryGetBSR.cpp
//  Description:    QueryGetBSR
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

#include "DBAccess/Queries/QueryGetBSR.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetBSRSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetBSR::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetBSR"));
std::string QueryGetBSR::_baseSQL;
bool QueryGetBSR::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetBSR> g_GetBSR;

const char*
QueryGetBSR::getQueryName() const
{
  return "GETBSR";
}

void
QueryGetBSR::initialize()
{
  if (!_isInitialized)
  {
    QueryGetBSRSQLStatement<QueryGetBSR> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETBSR");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetBSR::findBSR(std::vector<tse::BankerSellRate*>& BSRs, const CurrencyCode& primeCurrCode)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  DateTime date = DateTime::localTime().date();
  date = date.subtractDays(1);
  substParm(1, primeCurrCode);
  substParm(2, date);
  //	substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    BSRs.push_back(QueryGetBSRSQLStatement<QueryGetBSR>::mapRowToBSR(row));
  }
  LOG4CXX_INFO(_logger,
               "GETBSR: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                    << stopCPU() << ")");
  res.freeResult();
} // findBSR()

///////////////////////////////////////////////////////////
//
//  QueryGetBSRHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetBSRHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetBSRHistorical"));
std::string QueryGetBSRHistorical::_baseSQL;
bool QueryGetBSRHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetBSRHistorical> g_GetBSRHistorical;

const char*
QueryGetBSRHistorical::getQueryName() const
{
  return "GETBSRHISTORICAL";
}

void
QueryGetBSRHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetBSRHistoricalSQLStatement<QueryGetBSRHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETBSRHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetBSRHistorical::findBSR(std::vector<tse::BankerSellRate*>& BSRs,
                               const CurrencyCode& primeCurrCode,
                               const DateTime& startDate,
                               const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(primeCurrCode, 1);
  substParm(2, startDate);
  substParm(3, endDate);
  substParm(4, endDate);
  substParm(primeCurrCode, 5);
  substParm(6, startDate);
  substParm(7, endDate);
  substParm(8, endDate);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    BSRs.push_back(QueryGetBSRHistoricalSQLStatement<QueryGetBSRHistorical>::mapRowToBSR(row));
  }
  LOG4CXX_INFO(_logger,
               "GETBSRHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                              << stopCPU() << ")");
  res.freeResult();
} // findBSR()
}
