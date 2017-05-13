//-------------------------------------------------------------------------------
// (C) 2014, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc. Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetPassengerAirline.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetPassengerAirlineSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetPassengerAirline::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetPassengerAirline"));

std::string QueryGetPassengerAirline::_baseSQL;
bool QueryGetPassengerAirline::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetPassengerAirline> g_GetPassengerAirline;

const char*
QueryGetPassengerAirline::getQueryName() const
{
  return "GETPASSENGERAIRLINE";
}

void
QueryGetPassengerAirline::initialize()
{
  if (_isInitialized)
    return;

  QueryGetPassengerAirlineSQLStatement<QueryGetPassengerAirline> sqlStatement;
  _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETPASSENGERAIRLINE");
  substTableDef(&_baseSQL);
  _isInitialized = true;
}

void
QueryGetPassengerAirline::findPassengerAirline(
    std::vector<PassengerAirlineInfo*>& paiList,
    const CarrierCode& airlineCode)
{
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());

  substParm(1, airlineCode); // Insert airline code (%1q) parameter into SQL query string.

  DBResultSet res(_dbAdapt);
  res.executeQuery(this);

  PassengerAirlineInfo* pai;
  Row* row;
  while ((row = res.nextRow()))
  {
    pai = QueryGetPassengerAirlineSQLStatement<
        QueryGetPassengerAirline>::mapRowToPassengerAirlineInfo(row);
    paiList.push_back(pai);
  }

  LOG4CXX_INFO(_logger,
               getQueryName() << ": NumRows = " << res.numRows() << " Time = " << stopTimer()
                              << " (" << stopCPU() << ")");
  res.freeResult();
}

const QueryGetPassengerAirline&
QueryGetPassengerAirline::
operator=(const QueryGetPassengerAirline& rhs)
{
  if (this != &rhs)
  {
    *((SQLQuery*)this) = (SQLQuery&)rhs;
  }
  return *this;
}

const QueryGetPassengerAirline&
QueryGetPassengerAirline::
operator=(const std::string& rhs)
{
  if (this != &rhs)
  {
    *((SQLQuery*)this) = rhs;
  }
  return *this;
}

//-----------------------------------------------------------------------------
// HISTORICAL
//-----------------------------------------------------------------------------

log4cxx::LoggerPtr
QueryGetPassengerAirlineHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetPassengerAirlineHistorical"));

std::string QueryGetPassengerAirlineHistorical::_baseSQL;
bool QueryGetPassengerAirlineHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetPassengerAirlineHistorical> g_GetPassengerAirlineHistorical;

const char*
QueryGetPassengerAirlineHistorical::getQueryName() const
{
  return "GETPASSENGERAIRLINEHISTORICAL";
}

void
QueryGetPassengerAirlineHistorical::initialize()
{
  if (_isInitialized)
    return;

  QueryGetPassengerAirlineHistoricalSQLStatement<QueryGetPassengerAirlineHistorical> sqlStatement;
  _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETPASSENGERAIRLINEHISTORICAL");
  substTableDef(&_baseSQL);
  _isInitialized = true;
}

void
QueryGetPassengerAirlineHistorical::findPassengerAirline(
    std::vector<PassengerAirlineInfo*>& paiList,
    const CarrierCode& airlineCode,
    const DateTime& startDate,
    const DateTime& endDate)
{
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());

  substParm(1, airlineCode);
  substParm(2, startDate);
  substParm(3, endDate);

  DBResultSet res(_dbAdapt);
  res.executeQuery(this);

  PassengerAirlineInfo* pai;
  Row* row;
  while ((row = res.nextRow()))
  {
    pai = QueryGetPassengerAirlineSQLStatement<
        QueryGetPassengerAirlineHistorical>::mapRowToPassengerAirlineInfo(row);
    paiList.push_back(pai);
  }

  LOG4CXX_DEBUG(_logger, "startDate: " << startDate.toSimpleString() << "  endDate: " << endDate.toSimpleString());
  LOG4CXX_INFO(_logger,
               getQueryName() << ": NumRows = " << res.numRows() << " Time = " << stopTimer()
                              << " (" << stopCPU() << ")");
  res.freeResult();
}
}
