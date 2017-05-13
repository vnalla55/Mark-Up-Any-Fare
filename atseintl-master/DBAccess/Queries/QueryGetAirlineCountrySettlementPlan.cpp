//-------------------------------------------------------------------------------
// (C) 2014, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc. Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetAirlineCountrySettlementPlan.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetAirlineCountrySettlementPlanSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetAirlineCountrySettlementPlan::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAirlineCountrySettlementPlan"));

std::string QueryGetAirlineCountrySettlementPlan::_baseSQL;
bool QueryGetAirlineCountrySettlementPlan::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAirlineCountrySettlementPlan> g_GetAirlineCountrySettlementPlan;

const char*
QueryGetAirlineCountrySettlementPlan::getQueryName() const
{
  return "GETAIRLINECOUNTRYSETTLEMENTPLAN";
}

void
QueryGetAirlineCountrySettlementPlan::initialize()
{
  if (_isInitialized)
    return;

  QueryGetAirlineCountrySettlementPlanSQLStatement<QueryGetAirlineCountrySettlementPlan>
  sqlStatement;
  _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETAIRLINECOUNTRYSETTLEMENTPLAN");
  substTableDef(&_baseSQL);
  _isInitialized = true;
}

void
QueryGetAirlineCountrySettlementPlan::findAirlineCountrySettlementPlans(
    std::vector<tse::AirlineCountrySettlementPlanInfo*>& acspList,
    const NationCode& country,
    const CrsCode& gds,
    const CarrierCode& airline,
    const SettlementPlanType& spType)
{
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());

  substParm(1, country); // Insert country code (%1q) parameter into SQL query string.
  substParm(2, gds); // Insert GDS parameter (%2q) into SQL query string.
  substParm(3, airline); // Insert airline parameter (%3q) into SQL query string.
  substParm(4, spType); // Insert settlement plan type (%4q) parameter into SQL query string.

  DBResultSet res(_dbAdapt);
  res.executeQuery(this);

  Row* row;
  AirlineCountrySettlementPlanInfo* acsp;
  while ((row = res.nextRow()))
  {
    acsp = QueryGetAirlineCountrySettlementPlanSQLStatement<
        QueryGetAirlineCountrySettlementPlan>::mapRowToAirlineCountrySettlementPlan(row);
    acspList.push_back(acsp);
  }

  LOG4CXX_INFO(_logger,
               getQueryName() << ": NumRows = " << res.numRows() << " Time = " << stopTimer()
                              << " (" << stopCPU() << ")");
  res.freeResult();
}

const QueryGetAirlineCountrySettlementPlan&
QueryGetAirlineCountrySettlementPlan::
operator=(const QueryGetAirlineCountrySettlementPlan& rhs)
{
  if (this != &rhs)
  {
    *((SQLQuery*)this) = (SQLQuery&)rhs;
  }
  return *this;
}

const QueryGetAirlineCountrySettlementPlan&
QueryGetAirlineCountrySettlementPlan::
operator=(const std::string& rhs)
{
  if (this != &rhs)
  {
    *((SQLQuery*)this) = rhs;
  }
  return *this;
}

//-----------------------------------------------------------------------------
//  QueryGetAirlineCountrySettlementPlanHistorical
//-----------------------------------------------------------------------------

log4cxx::LoggerPtr
QueryGetAirlineCountrySettlementPlanHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAirlineCountrySettlementPlanHistorical"));

std::string QueryGetAirlineCountrySettlementPlanHistorical::_baseSQL;
bool QueryGetAirlineCountrySettlementPlanHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAirlineCountrySettlementPlanHistorical> g_GetAirlineCountrySettlementPlanHistorical;

const char*
QueryGetAirlineCountrySettlementPlanHistorical::getQueryName() const
{
  return "GETAIRLINECOUNTRYSETTLEMENTPLANHISTORICAL";
}

void
QueryGetAirlineCountrySettlementPlanHistorical::initialize()
{
  if (_isInitialized)
    return;

  QueryGetAirlineCountrySettlementPlanHistoricalSQLStatement<QueryGetAirlineCountrySettlementPlanHistorical>
  sqlStatement;
  _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETAIRLINECOUNTRYSETTLEMENTPLANHISTORICAL");
  substTableDef(&_baseSQL);
  _isInitialized = true;

}

void
QueryGetAirlineCountrySettlementPlanHistorical::findAirlineCountrySettlementPlans(
    std::vector<tse::AirlineCountrySettlementPlanInfo*>& acspList,
    const NationCode& country,
    const CrsCode& gds,
    const CarrierCode& airline,
    const SettlementPlanType& spType,
    const DateTime& startDate,
    const DateTime& endDate)
{
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());

  substParm(1, country);
  substParm(2, gds);
  substParm(3, airline);
  substParm(4, spType);
  substParm(5, startDate);
  substParm(6, endDate);

  DBResultSet res(_dbAdapt);
  res.executeQuery(this);

  Row* row;
  AirlineCountrySettlementPlanInfo* acsp;
  while ((row = res.nextRow()))
  {
    acsp = QueryGetAirlineCountrySettlementPlanSQLStatement<
        QueryGetAirlineCountrySettlementPlanHistorical>::mapRowToAirlineCountrySettlementPlan(row);
    acspList.push_back(acsp);
  }

  LOG4CXX_DEBUG(_logger, "startDate: " << startDate.toSimpleString() << "  endDate: " << endDate.toSimpleString());
  LOG4CXX_INFO(_logger,
               getQueryName() << ": NumRows = " << res.numRows() << " Time = " << stopTimer()
                              << " (" << stopCPU() << ")");
  res.freeResult();
}

//-----------------------------------------------------------------------------
//  QueryGetAllAirlineCountrySettlementPlans
//-----------------------------------------------------------------------------

log4cxx::LoggerPtr
QueryGetAllAirlineCountrySettlementPlans::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetAllAirlineCountrySettlementPlans"));

std::string QueryGetAllAirlineCountrySettlementPlans::_baseSQL;
bool QueryGetAllAirlineCountrySettlementPlans::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllAirlineCountrySettlementPlans> g_GetAllAirlineCountrySettlementPlans;

const char*
QueryGetAllAirlineCountrySettlementPlans::getQueryName() const
{
  return "GETALLAIRLINECOUNTRYSETTLEMENTPLANS";
}

void
QueryGetAllAirlineCountrySettlementPlans::initialize()
{
  if (_isInitialized)
    return;

  QueryGetAllAirlineCountrySettlementPlanSQLStatement<QueryGetAllAirlineCountrySettlementPlans>
  sqlStatement;
  _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLAIRLINECOUNTRYSETTLEMENTPLANS");
  substTableDef(&_baseSQL);
  _isInitialized = true;

}

void
QueryGetAllAirlineCountrySettlementPlans::findAirlineCountrySettlementPlans(
    std::vector<tse::AirlineCountrySettlementPlanInfo*>& acspList)
{
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());

  substCurrentDate();
  DBResultSet res(_dbAdapt);
  res.executeQuery(this);

  Row* row;
  AirlineCountrySettlementPlanInfo* acsp;
  while ((row = res.nextRow()))
  {
    acsp = QueryGetAirlineCountrySettlementPlanSQLStatement<
        QueryGetAllAirlineCountrySettlementPlans>::mapRowToAirlineCountrySettlementPlan(row);
    acspList.push_back(acsp);
  }

  LOG4CXX_INFO(_logger,
               getQueryName() << ": NumRows = " << res.numRows() << " Time = " << stopTimer()
                              << " (" << stopCPU() << ")");
  res.freeResult();
}
}
