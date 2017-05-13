//-------------------------------------------------------------------
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//-------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetFrequentFlyerStatus.h"

#include "Common/Logger.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/FreqFlyerStatus.h"
#include "DBAccess/Queries/QueryGetFrequentFlyerStatusSQLStatement.h"

namespace tse
{
static const char* queryName = "GETFREQUENTFLYERSTATUS";
static const char* queryNameHistorical = "GETFREQUENTFLYERSTATUSHISTORICAL";
Logger
QueryGetFrequentFlyerStatus::_logger("atseintl.DBAccess.SQLQuery.GetFrequentFlyerStatus");
std::string QueryGetFrequentFlyerStatus::_baseSQL;
bool QueryGetFrequentFlyerStatus::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFrequentFlyerStatus> g_GetFrequentFlyerStatus;

const char*
QueryGetFrequentFlyerStatus::getQueryName() const
{
  return queryName;
}

std::vector<FreqFlyerStatus*>
QueryGetFrequentFlyerStatus::findStatus(const CarrierCode carrier)
{
  DBResultSet res(_dbAdapt);

  substParm(carrier, 1);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());

  res.executeQuery(this);
  LOG4CXX_INFO(_logger,
               "getFreqFlyerStatusSegs: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                    << " (" << stopCPU() << ")");

  Row* row;
  std::vector<FreqFlyerStatus*> statuses;
  while ((row = res.nextRow()))
    statuses.push_back(
        QueryGetFrequentFlyerStatusSQLStatement<QueryGetFrequentFlyerStatus>::mapRowToStatus(row));

  res.freeResult();

  return statuses;
}

void
QueryGetFrequentFlyerStatus::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFrequentFlyerStatusSQLStatement<QueryGetFrequentFlyerStatus> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL(queryName);
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

Logger
QueryGetFrequentFlyerStatusHistorical::_logger(
    "atseintl.DBAccess.SQLQuery.GetFrequentFlyerStatusHistorical");
std::string QueryGetFrequentFlyerStatusHistorical::_baseSQL;
bool QueryGetFrequentFlyerStatusHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFrequentFlyerStatusHistorical> g_GetFrequentFlyerStatusHistorical;

const char*
QueryGetFrequentFlyerStatusHistorical::getQueryName() const
{
  return queryNameHistorical;
}

std::vector<FreqFlyerStatus*>
QueryGetFrequentFlyerStatusHistorical::findStatus(const CarrierCode carrier,
                                                  const DateTime& startDate,
                                                  const DateTime& endDate)
{
  DBResultSet res(_dbAdapt);

  substParm(carrier, 1);
  substParm(2, startDate);
  substParm(3, endDate);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());

  res.executeQuery(this);
  LOG4CXX_INFO(_logger,
               "GETFREQFLYERSTATUSHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");

  Row* row;
  std::vector<FreqFlyerStatus*> statuses;
  while ((row = res.nextRow()))
    statuses.push_back(QueryGetFrequentFlyerStatusHistoricalSQLStatement<
        QueryGetFrequentFlyerStatusHistorical>::mapRowToStatus(row));

  res.freeResult();

  return statuses;
}

void
QueryGetFrequentFlyerStatusHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFrequentFlyerStatusHistoricalSQLStatement<QueryGetFrequentFlyerStatusHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL(queryNameHistorical);
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}
}
