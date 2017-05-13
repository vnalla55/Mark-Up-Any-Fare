//-------------------------------------------------------------------------------
// (C) 2014, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc. Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetCountrySettlementPlan.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCountrySettlementPlanSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetCountrySettlementPlan::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetCountrySettlementPlan"));

std::string QueryGetCountrySettlementPlan::_baseSQL;
bool QueryGetCountrySettlementPlan::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCountrySettlementPlan> g_GetCountrySettlementPlan;

const char*
QueryGetCountrySettlementPlan::getQueryName() const
{
  return "GETCOUNTRYSETTLEMENTPLAN";
}

void
QueryGetCountrySettlementPlan::initialize()
{
  if (_isInitialized)
    return;

  QueryGetCountrySettlementPlanSQLStatement<QueryGetCountrySettlementPlan> sqlStatement;
  _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCOUNTRYSETTLEMENTPLAN");
  substTableDef(&_baseSQL);
  _isInitialized = true;
}

void
QueryGetCountrySettlementPlan::findCountrySettlementPlan(
    std::vector<tse::CountrySettlementPlanInfo*>& countrySettlementPlans,
    const NationCode& countryCode)
{
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());

  substParm(1, countryCode); // Insert country code parameter into SQL query string.

  DBResultSet res(_dbAdapt);
  res.executeQuery(this);

  Row* row;
  CountrySettlementPlanInfo* info;
  while ((row = res.nextRow()))
  {
    info = QueryGetCountrySettlementPlanSQLStatement<
        QueryGetCountrySettlementPlan>::mapRowToCountrySettlementPlanInfo(row);
    countrySettlementPlans.push_back(info);
  }

  LOG4CXX_INFO(_logger,
               getQueryName() << ": NumRows = " << res.numRows() << " Time = " << stopTimer()
                              << " (" << stopCPU() << ")");
  res.freeResult();
}

const QueryGetCountrySettlementPlan&
QueryGetCountrySettlementPlan::
operator=(const QueryGetCountrySettlementPlan& rhs)
{
  if (this != &rhs)
  {
    *((SQLQuery*)this) = (SQLQuery&)rhs;
  }
  return *this;
}

const QueryGetCountrySettlementPlan&
QueryGetCountrySettlementPlan::
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
QueryGetCountrySettlementPlanHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetCountrySettlementPlanHistorical"));

std::string QueryGetCountrySettlementPlanHistorical::_baseSQL;
bool QueryGetCountrySettlementPlanHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCountrySettlementPlanHistorical> g_GetCountrySettlementPlanHistorical;

const char*
QueryGetCountrySettlementPlanHistorical::getQueryName() const
{
  return "GETCOUNTRYSETTLEMENTPLANHISTORICAL";
}

void
QueryGetCountrySettlementPlanHistorical::initialize()
{
  if (_isInitialized)
    return;

  QueryGetCountrySettlementPlanHistoricalSQLStatement<QueryGetCountrySettlementPlanHistorical> sqlStatement;
  _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCOUNTRYSETTLEMENTPLANHISTORICAL");
  substTableDef(&_baseSQL);
  _isInitialized = true;
}

void
QueryGetCountrySettlementPlanHistorical::findCountrySettlementPlan(
    std::vector<tse::CountrySettlementPlanInfo*>& cspList,
    const NationCode& countryCode,
    const DateTime& startDate,
    const DateTime& endDate)
{
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());

  substParm(1, countryCode);
  substParm(2, startDate);
  substParm(3, endDate);

  DBResultSet res(_dbAdapt);
  res.executeQuery(this);

  Row* row;
  CountrySettlementPlanInfo* info;
  while ((row = res.nextRow()))
  {
    info = QueryGetCountrySettlementPlanSQLStatement<
        QueryGetCountrySettlementPlanHistorical>::mapRowToCountrySettlementPlanInfo(row);
    cspList.push_back(info);
  }

  LOG4CXX_DEBUG(_logger, "startDate: " << startDate.toSimpleString() << "  endDate: " << endDate.toSimpleString());
  LOG4CXX_INFO(_logger,
               getQueryName() << ": NumRows = " << res.numRows() << " Time = " << stopTimer()
                              << " (" << stopCPU() << ")");
  res.freeResult();
}
}
