//-------------------------------------------------------------------
//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetMaxPermittedAmountFare.h"

#include "Common/Global.h"
#include "Common/Logger.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetMaxPermittedAmountFareSQLStatement.h"

namespace tse
{
namespace
{
Logger
gLogger("atseintl.DBAccess.SQLQuery.QueryGetMaxPermittedAmountFare");
Logger
gLoggerH("atseintl.DBAccess.SQLQuery.QueryGetMaxPermittedAmountFareHistorical");
}

std::string QueryGetMaxPermittedAmountFare::_baseSQL;
bool
QueryGetMaxPermittedAmountFare::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetMaxPermittedAmountFare> _getMaxPermittedAmountFare;

const char*
QueryGetMaxPermittedAmountFare::getQueryName() const
{
  return "GETMAXPERMITTEDAMOUNTFARE";
}

void
QueryGetMaxPermittedAmountFare::initialize()
{
  if (_isInitialized)
    return;

  QueryGetMaxPermittedAmountFareSQLStatement<QueryGetMaxPermittedAmountFare> sqlStatement;
  _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMAXPERMITTEDAMOUNTFARE");
  substTableDef(&_baseSQL);

  _isInitialized = true;
}

void
QueryGetMaxPermittedAmountFare::findMaxPermittedAmountFare(
    std::vector<MaxPermittedAmountFareInfo*>& lst, const MaxPermittedAmountFareKey& key)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  const LocCode& originAirport = key._a;
  const LocCode& originCity = key._b;
  const NationCode& originNation = key._c;
  const LocCode& destAirport = key._d;
  const LocCode& destCity = key._e;
  const NationCode& destNation = key._f;

  substParm(originAirport, 1);
  substParm(originCity, 2);
  substParm(originNation, 3);
  substParm(destAirport, 4);
  substParm(destCity, 5);
  substParm(destNation, 6);
  substParm(originAirport, 7);
  substParm(originCity, 8);
  substParm(originNation, 9);
  substParm(destAirport, 10);
  substParm(destCity, 11);
  substParm(destNation, 12);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(gLogger, startTimer());
  res.executeQuery(this);

  int rows = res.numRows();
  if (LIKELY(rows > 0))
    lst.reserve(rows);

  while ((row = res.nextRow()))
  {
    lst.push_back(
        QueryGetMaxPermittedAmountFareSQLStatement<QueryGetMaxPermittedAmountFare>::mapRow(row));
  }
  LOG4CXX_INFO(gLogger,
               "GETMAXPERMITTEDAMOUNTFARE: NumRows: " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ") ms");
  res.freeResult();
}

std::string QueryGetMaxPermittedAmountFareHistorical::_baseSQL;
bool QueryGetMaxPermittedAmountFareHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMaxPermittedAmountFareHistorical>
_getMaxPermittedAmountFareHistorical;

const char*
QueryGetMaxPermittedAmountFareHistorical::getQueryName() const
{
  return "GETMAXPERMITTEDAMOUNTFAREHISTORICAL";
}

void
QueryGetMaxPermittedAmountFareHistorical::initialize()
{
  if (_isInitialized)
    return;

  QueryGetMaxPermittedAmountFareHistoricalSQLStatement<QueryGetMaxPermittedAmountFareHistorical>
  sqlStatement;
  _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMAXPERMITTEDAMOUNTFAREHISTORICAL");
  substTableDef(&_baseSQL);
  _isInitialized = true;
} // initialize()

void
QueryGetMaxPermittedAmountFareHistorical::findMaxPermittedAmountFare(
    std::vector<MaxPermittedAmountFareInfo*>& lst, const MaxPermittedAmountFareHistoricalKey& key)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  const LocCode& originAirport = key._a;
  const LocCode& originCity = key._b;
  const NationCode& originNation = key._c;
  const LocCode& destAirport = key._d;
  const LocCode& destCity = key._e;
  const NationCode& destNation = key._f;
  const DateTime& startDate = key._g;
  const DateTime& endDate = key._h;

  substParm(originAirport, 1);
  substParm(originCity, 2);
  substParm(originNation, 3);
  substParm(destAirport, 4);
  substParm(destCity, 5);
  substParm(destNation, 6);
  substParm(originAirport, 7);
  substParm(originCity, 8);
  substParm(originNation, 9);
  substParm(destAirport, 10);
  substParm(destCity, 11);
  substParm(destNation, 12);
  substParm(13, startDate);
  substParm(14, endDate);
  LOG4CXX_EXECUTECODE_INFO(gLoggerH, startTimer());
  res.executeQuery(this);

  int rows = res.numRows();
  if (LIKELY(rows > 0))
    lst.reserve(rows);

  while ((row = res.nextRow()))
  {
    lst.push_back(QueryGetMaxPermittedAmountFareHistoricalSQLStatement<
        QueryGetMaxPermittedAmountFareHistorical>::mapRow(row));
  }
  LOG4CXX_INFO(gLoggerH,
               "GETMAXPERMITTEDAMOUNTFAREHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
}
} // tse
