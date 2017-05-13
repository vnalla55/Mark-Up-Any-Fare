// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#include "DBAccess/Queries/QueryGetAirlineAllianceContinent.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetAirlineAllianceContinentSQLStatement.h"

namespace tse
{
///////////////////////////////////////////////////////////
//  QueryGetAirlineAllianceContinent
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAirlineAllianceContinent::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAirlineAllianceContinent"));
std::string QueryGetAirlineAllianceContinent::_baseSQL;
bool QueryGetAirlineAllianceContinent::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAirlineAllianceContinent> g_GetAirlineAllianceContinent;

const char*
QueryGetAirlineAllianceContinent::getQueryName() const
{
  return "GETAIRLINEALLIANCECONTINENT";
}

void
QueryGetAirlineAllianceContinent::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAirlineAllianceContinentSQLStatement<QueryGetAirlineAllianceContinent> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETAIRLINEALLIANCECONTINENT");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetAirlineAllianceContinent::findAirlineAllianceContinentInfo(
    std::vector<tse::AirlineAllianceContinentInfo*>& airlineAllianceContinentInfovec,
    const GenericAllianceCode& genericAllianceCode)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(genericAllianceCode, 1);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    airlineAllianceContinentInfovec.push_back(QueryGetAirlineAllianceContinentSQLStatement<
        QueryGetAirlineAllianceContinent>::mapRowToAirlineAllianceContinentInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETAIRLINEALLIANCECONTINENT: NumRows " << res.numRows() << " Time = " << stopTimer()
                                                       << " (" << stopCPU() << ") mSecs");
  res.freeResult();
}

///////////////////////////////////////////////////////////
//  QueryGetAirlineAllianceContinentHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAirlineAllianceContinentHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.QueryGetAirlineAllianceContinentHistorical"));
std::string QueryGetAirlineAllianceContinentHistorical::_baseSQL;
bool QueryGetAirlineAllianceContinentHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAirlineAllianceContinentHistorical>
g_GETAIRLINEALLIANCECONTINENTHISTORICAL;

const char*
QueryGetAirlineAllianceContinentHistorical::getQueryName() const
{
  return "GETAIRLINEALLIANCECONTINENTHISTORICAL";
}

void
QueryGetAirlineAllianceContinentHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAirlineAllianceContinentHistoricalSQLStatement<
        QueryGetAirlineAllianceContinentHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETAIRLINEALLIANCECONTINENTHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetAirlineAllianceContinentHistorical::findAirlineAllianceContinentInfo(
    std::vector<tse::AirlineAllianceContinentInfo*>& airlineAllianceContinentInfovec,
    const GenericAllianceCode& genericAllianceCode,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(genericAllianceCode, 1);
  substParm(2, startDate);
  substParm(3, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    airlineAllianceContinentInfovec.push_back(
        QueryGetAirlineAllianceContinentHistoricalSQLStatement<
            QueryGetAirlineAllianceContinentHistorical>::mapRowToAirlineAllianceContinentInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETAIRLINEALLIANCECONTINENTHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
}
}
