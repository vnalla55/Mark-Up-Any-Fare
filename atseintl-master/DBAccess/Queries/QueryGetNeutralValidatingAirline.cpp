//-------------------------------------------------------------------------------
// (C) 2014, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc. Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetNeutralValidatingAirline.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetNeutralValidatingAirlineSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetNeutralValidatingAirline::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetNeutralValidatingAirline"));

std::string QueryGetNeutralValidatingAirline::_baseSQL;
bool QueryGetNeutralValidatingAirline::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetNeutralValidatingAirline> g_GetNeutralValidatingAirline;

const char*
QueryGetNeutralValidatingAirline::getQueryName() const
{
  return "GETNEUTRALVALIDATINGAIRLINE";
}

void
QueryGetNeutralValidatingAirline::initialize()
{
  if (_isInitialized)
    return;

  QueryGetNeutralValidatingAirlineSQLStatement<QueryGetNeutralValidatingAirline> sqlStatement;
  _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETNEUTRALVALIDATINGAIRLINE");
  substTableDef(&_baseSQL);
  _isInitialized = true;
}

void
QueryGetNeutralValidatingAirline::findNeutralValidatingAirlineInfo(
    std::vector<tse::NeutralValidatingAirlineInfo*>& nvaList,
    const NationCode& country,
    const CrsCode& gds,
    const SettlementPlanType& spType)
{
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());

  substParm(1, country); // Insert country code (%1q) parameter into SQL query string.
  substParm(2, gds); // Insert GDS parameter (%2q) into SQL query string.
  substParm(3, spType); // Insert settlement plan type (%3q) parameter into SQL query string.

  DBResultSet res(_dbAdapt);
  res.executeQuery(this);

  Row* row;
  NeutralValidatingAirlineInfo* nva;
  while ((row = res.nextRow()))
  {
    nva = QueryGetNeutralValidatingAirlineSQLStatement<
        QueryGetNeutralValidatingAirline>::mapRowToNeutralValidatingAirlineInfo(row);
    nvaList.push_back(nva);
  }

  LOG4CXX_INFO(_logger,
               getQueryName() << ": NumRows = " << res.numRows() << " Time = " << stopTimer()
                              << " (" << stopCPU() << ")");
  res.freeResult();
}

const QueryGetNeutralValidatingAirline&
QueryGetNeutralValidatingAirline::
operator=(const QueryGetNeutralValidatingAirline& rhs)
{
  if (this != &rhs)
  {
    *((SQLQuery*)this) = (SQLQuery&)rhs;
  }
  return *this;
}

const QueryGetNeutralValidatingAirline&
QueryGetNeutralValidatingAirline::
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
QueryGetNeutralValidatingAirlineHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetNeutralValidatingAirlineHistorical"));

std::string QueryGetNeutralValidatingAirlineHistorical::_baseSQL;
bool QueryGetNeutralValidatingAirlineHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetNeutralValidatingAirlineHistorical> g_GetNeutralValidatingAirlineHistorical;

const char*
QueryGetNeutralValidatingAirlineHistorical::getQueryName() const
{
  return "GETNEUTRALVALIDATINGAIRLINEHISTORICAL";
}

void
QueryGetNeutralValidatingAirlineHistorical::initialize()
{
  if (_isInitialized)
    return;

  QueryGetNeutralValidatingAirlineHistoricalSQLStatement<
      QueryGetNeutralValidatingAirlineHistorical> sqlStatement;
  _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETNEUTRALVALIDATINGAIRLINEHISTORICAL");
  substTableDef(&_baseSQL);
  _isInitialized = true;
}

void
QueryGetNeutralValidatingAirlineHistorical::findNeutralValidatingAirlineInfo(
    std::vector<tse::NeutralValidatingAirlineInfo*>& nvaList,
    const NationCode& country,
    const CrsCode& gds,
    const SettlementPlanType& spType,
    const DateTime& startDate,
    const DateTime& endDate)
{
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());

  substParm(1, country);
  substParm(2, gds);
  substParm(3, spType);
  substParm(4, startDate);
  substParm(5, endDate);

  DBResultSet res(_dbAdapt);
  res.executeQuery(this);

  Row* row;
  NeutralValidatingAirlineInfo* nva;
  while ((row = res.nextRow()))
  {
    nva = QueryGetNeutralValidatingAirlineHistoricalSQLStatement<
        QueryGetNeutralValidatingAirlineHistorical>::mapRowToNeutralValidatingAirlineInfo(row);
    nvaList.push_back(nva);
  }

  LOG4CXX_DEBUG(_logger, "startDate: " << startDate.toSimpleString() << "  endDate: " << endDate.toSimpleString());
  LOG4CXX_INFO(_logger,
               getQueryName() << ": NumRows = " << res.numRows() << " Time = " << stopTimer()
                              << " (" << stopCPU() << ")");
  res.freeResult();
}
}
