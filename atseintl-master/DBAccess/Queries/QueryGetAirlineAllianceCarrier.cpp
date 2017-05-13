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

#include "DBAccess/Queries/QueryGetAirlineAllianceCarrier.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetAirlineAllianceCarrierSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetAirlineAllianceCarrier::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAirlineAllianceCarrier"));
std::string QueryGetAirlineAllianceCarrier::_baseSQL;
bool QueryGetAirlineAllianceCarrier::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAirlineAllianceCarrier> g_GetAirlineAllianceCarrier;

const char*
QueryGetAirlineAllianceCarrier::getQueryName() const
{
  return "GETAIRLINEALLIANCECARRIER";
}

void
QueryGetAirlineAllianceCarrier::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAirlineAllianceCarrierSQLStatement<QueryGetAirlineAllianceCarrier> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETAIRLINEALLIANCECARRIER");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetAirlineAllianceCarrier::findAirlineAllianceCarrierInfo(
    std::vector<tse::AirlineAllianceCarrierInfo*>& airlineAllianceCarrierInfovec,
    const CarrierCode& carrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(carrier, 1);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    airlineAllianceCarrierInfovec.push_back(QueryGetAirlineAllianceCarrierSQLStatement<
        QueryGetAirlineAllianceCarrier>::mapRowToAirlineAllianceCarrierInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETAIRLINEALLIANCECARRIER: NumRows " << res.numRows() << " Time = " << stopTimer()
                                                     << " (" << stopCPU() << ") mSecs");
  res.freeResult();
}

///////////////////////////////////////////////////////////
//  QueryGetAirlineAllianceCarrierHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAirlineAllianceCarrierHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.QueryGetAirlineAllianceCarrierHistorical"));
std::string QueryGetAirlineAllianceCarrierHistorical::_baseSQL;
bool QueryGetAirlineAllianceCarrierHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAirlineAllianceCarrierHistorical>
g_GETAIRLINEALLIANCECARRIERHISTORICAL;

const char*
QueryGetAirlineAllianceCarrierHistorical::getQueryName() const
{
  return "GETAIRLINEALLIANCECARRIERHISTORICAL";
}

void
QueryGetAirlineAllianceCarrierHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAirlineAllianceCarrierHistoricalSQLStatement<QueryGetAirlineAllianceCarrierHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETAIRLINEALLIANCECARRIERHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetAirlineAllianceCarrierHistorical::findAirlineAllianceCarrierInfo(
    std::vector<tse::AirlineAllianceCarrierInfo*>& airlineAllianceCarrierInfovec,
    const CarrierCode& carrier,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(carrier, 1);
  substParm(2, startDate);
  substParm(3, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    airlineAllianceCarrierInfovec.push_back(QueryGetAirlineAllianceCarrierHistoricalSQLStatement<
        QueryGetAirlineAllianceCarrierHistorical>::mapRowToAirlineAllianceCarrierInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETAIRLINEALLIANCECARRIERHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
}
}
