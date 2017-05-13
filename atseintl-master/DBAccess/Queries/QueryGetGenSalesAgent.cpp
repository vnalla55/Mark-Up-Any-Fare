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

#include "DBAccess/Queries/QueryGetGenSalesAgent.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/GenSalesAgentInfo.h"
#include "DBAccess/Queries/QueryGetGenSalesAgentSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetGenSalesAgent::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetGenSalesAgent"));

std::string QueryGetGenSalesAgent::_baseSQL;
bool QueryGetGenSalesAgent::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetGenSalesAgent> g_GetGeneralSalesAgent;

const char*
QueryGetGenSalesAgent::getQueryName() const
{
  return "GETGENSALESAGENT";
}

void
QueryGetGenSalesAgent::initialize()
{
  if (!_isInitialized)
  {
    QueryGetGenSalesAgentSQLStatement<QueryGetGenSalesAgent> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETGENSALESAGENT");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

const QueryGetGenSalesAgent&
QueryGetGenSalesAgent::
operator=(const QueryGetGenSalesAgent& rhs)
{
  if (this != &rhs)
    *((SQLQuery*)this) = (SQLQuery&)rhs;
  return *this;
};

const QueryGetGenSalesAgent&
QueryGetGenSalesAgent::
operator=(const std::string& rhs)
{
  if (this != &rhs)
    *((SQLQuery*)this) = rhs;
  return *this;
}

void
QueryGetGenSalesAgent::findGenSalesAgents(std::vector<tse::GenSalesAgentInfo*>& genSalesAgents,
                                          const CrsCode& gds,
                                          const NationCode& country,
                                          const SettlementPlanType& settlementPlan,
                                          const CarrierCode& validatingCxr)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(gds, 1);
  substParm(country, 2);
  substParm(settlementPlan, 3);
  substParm(validatingCxr, 4);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    genSalesAgents.push_back(
        QueryGetGenSalesAgentSQLStatement<QueryGetGenSalesAgent>::mapRowToGenSalesAgentInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETGENSALESAGENT: NumRows " << res.numRows() << " Time = " << stopTimer() << " ("
                                            << stopCPU() << ") mSecs");
  res.freeResult();
}

//-----------------------------------------------------------------------------
// HISTORICAL
//-----------------------------------------------------------------------------

log4cxx::LoggerPtr
QueryGetGenSalesAgentHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetGenSalesAgentHistorical"));

std::string QueryGetGenSalesAgentHistorical::_baseSQL;
bool QueryGetGenSalesAgentHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetGenSalesAgentHistorical> g_GetGeneralSalesAgentHistorical;

const char*
QueryGetGenSalesAgentHistorical::getQueryName() const
{
  return "GETGENSALESAGENTHISTORICAL";
}

void
QueryGetGenSalesAgentHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetGenSalesAgentHistoricalSQLStatement<QueryGetGenSalesAgentHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETGENSALESAGENTHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetGenSalesAgentHistorical::findGenSalesAgents(std::vector<tse::GenSalesAgentInfo*>& genSalesAgents,
                                                    const CrsCode& gds,
                                                    const NationCode& country,
                                                    const SettlementPlanType& settlementPlan,
                                                    const CarrierCode& validatingCxr,
                                                    const DateTime& startDate,
                                                    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, gds);
  substParm(2, country);
  substParm(3, settlementPlan);
  substParm(4, validatingCxr);
  substParm(5, startDate);
  substParm(6, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    genSalesAgents.push_back(
        QueryGetGenSalesAgentSQLStatement<QueryGetGenSalesAgentHistorical>::mapRowToGenSalesAgentInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETGENSALESAGENTHISTORICAL: NumRows " << res.numRows() << " Time = " << stopTimer() << " ("
                                            << stopCPU() << ") mSecs");
  res.freeResult();
}

//-----------------------------------------------------------------------------
//  QueryGetAllGenSalesAgents
//-----------------------------------------------------------------------------

log4cxx::LoggerPtr
QueryGetAllGenSalesAgents::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetAllGenSalesAgents"));

std::string QueryGetAllGenSalesAgents::_baseSQL;
bool QueryGetAllGenSalesAgents::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllGenSalesAgents> g_GetAllGenSalesAgents;

const char*
QueryGetAllGenSalesAgents::getQueryName() const
{
  return "GETALLGENSALESAGENTS";
}

void
QueryGetAllGenSalesAgents::initialize()
{
  if (_isInitialized)
    return;

  QueryGetAllGenSalesAgentSQLStatement<QueryGetAllGenSalesAgents>
  sqlStatement;
  _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLGENSALESAGENTS");
  substTableDef(&_baseSQL);
  _isInitialized = true;

}

void
QueryGetAllGenSalesAgents::findAllGenSalesAgents(std::vector<tse::GenSalesAgentInfo*>& gsaList)
{
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());

  substCurrentDate();
  DBResultSet res(_dbAdapt);
  res.executeQuery(this);

  Row* row;
  GenSalesAgentInfo* gsa;
  while ((row = res.nextRow()))
  {
    gsa = QueryGetGenSalesAgentSQLStatement<QueryGetAllGenSalesAgents>::mapRowToGenSalesAgentInfo(row);
    gsaList.push_back(gsa);
  }

  LOG4CXX_INFO(_logger,
               getQueryName() << ": NumRows = " << res.numRows() << " Time = " << stopTimer()
                              << " (" << stopCPU() << ")");
  res.freeResult();
}
}
