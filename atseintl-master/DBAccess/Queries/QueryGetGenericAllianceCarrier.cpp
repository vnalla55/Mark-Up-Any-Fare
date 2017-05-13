// ----------------------------------------------------------------
//
//   Copyright Sabre 2014
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

#include "DBAccess/Queries/QueryGetGenericAllianceCarrier.h"

#include "Common/Global.h"
#include "Common/Logger.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetGenericAllianceCarrierSQLStatement.h"

namespace tse
{

///////////////////////////////////////////////////////////
//  QueryGetGenericAllianceCarrier
///////////////////////////////////////////////////////////
Logger
QueryGetGenericAllianceCarrier::_logger("atseintl.DBAccess.SQLQuery.GetGenericAllianceCarrier");
std::string QueryGetGenericAllianceCarrier::_baseSQL;
bool QueryGetGenericAllianceCarrier::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetGenericAllianceCarrier> g_GetGenericAllianceCarrier;

const char*
QueryGetGenericAllianceCarrier::getQueryName() const
{
  return "GETGENERICALLIANCECARRIER";
}

void
QueryGetGenericAllianceCarrier::initialize()
{
  if (!_isInitialized)
  {
    QueryGetGenericAllianceCarrierSQLStatement<QueryGetGenericAllianceCarrier> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETGENERICALLIANCECARRIER");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetGenericAllianceCarrier::findGenericAllianceCarrierInfo(
    std::vector<tse::AirlineAllianceCarrierInfo*>& airlineAllianceCarrierInfovec,
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
    airlineAllianceCarrierInfovec.push_back(QueryGetGenericAllianceCarrierSQLStatement<
        QueryGetGenericAllianceCarrier>::mapRowToGenericAllianceCarrierInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETGENERICALLIANCECARRIER: NumRows " << res.numRows() << " Time = " << stopTimer()
                                                     << " (" << stopCPU() << ") mSecs");
  res.freeResult();
}

Logger
QueryGetGenericAllianceCarrierHistorical::_logger(
    "atseintl.DBAccess.SQLQuery.QueryGetGenericAllianceCarrierHistorical");
std::string QueryGetGenericAllianceCarrierHistorical::_baseSQL;
bool QueryGetGenericAllianceCarrierHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetGenericAllianceCarrierHistorical>
g_GETGENERICALLIANCECARRIERHISTORICAL;

const char*
QueryGetGenericAllianceCarrierHistorical::getQueryName() const
{
  return "GETGENERICALLIANCECARRIERHISTORICAL";
}

void
QueryGetGenericAllianceCarrierHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetGenericAllianceCarrierHistoricalSQLStatement<QueryGetGenericAllianceCarrierHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETGENERICALLIANCECARRIERHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetGenericAllianceCarrierHistorical::findGenericAllianceCarrierInfo(
    std::vector<tse::AirlineAllianceCarrierInfo*>& airlineAllianceCarrierInfovec,
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
    airlineAllianceCarrierInfovec.push_back(QueryGetGenericAllianceCarrierHistoricalSQLStatement<
        QueryGetGenericAllianceCarrierHistorical>::mapRowToGenericAllianceCarrierInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETGENERICALLIANCECARRIERHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
}

}//tse
