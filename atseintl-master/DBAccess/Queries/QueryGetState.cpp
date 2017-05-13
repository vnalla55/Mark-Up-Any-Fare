//----------------------------------------------------------------------------
//  File:           QueryGetState.cpp
//  Description:    QueryGetState
//  Created:        5/24/2006
// Authors:         Mike Lillis
//
//  Updates:
//
// � 2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetState.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetStateSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetState::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetState"));
std::string QueryGetState::_baseSQL;
bool QueryGetState::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetState> g_GetState;

const char*
QueryGetState::getQueryName() const
{
  return "GETSTATE";
}

void
QueryGetState::initialize()
{
  if (!_isInitialized)
  {
    QueryGetStateSQLStatement<QueryGetState> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSTATE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetState::findState(std::vector<tse::State*>& states,
                         const NationCode& nation,
                         const StateCode& state)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, nation);
  substParm(2, state);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    states.push_back(QueryGetStateSQLStatement<QueryGetState>::mapRowToState(row));
  }
  LOG4CXX_INFO(_logger,
               "GETSTATE: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                      << stopCPU() << ")");
  res.freeResult();
} // findState()

///////////////////////////////////////////////////////////
//  QueryGetStateHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetStateHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetStateHistorical"));
bool QueryGetStateHistorical::_isInitialized = false;
std::string QueryGetStateHistorical::_baseSQL;
SQLQueryInitializerHelper<QueryGetStateHistorical> g_GetStateHistorical;

const char*
QueryGetStateHistorical::getQueryName() const
{
  return "GETSTATEHISTORICAL";
}

void
QueryGetStateHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetStateHistoricalSQLStatement<QueryGetStateHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSTATEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetStateHistorical::findState(std::vector<tse::State*>& states,
                                   const NationCode& nation,
                                   const StateCode& state)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, nation);
  substParm(2, state);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    states.push_back(
        QueryGetStateHistoricalSQLStatement<QueryGetStateHistorical>::mapRowToState(row));
  }
  LOG4CXX_INFO(_logger,
               "GETSTATEHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetStateHistorical::findState()

///////////////////////////////////////////////////////////
//  QueryGetStates
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetStates::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetStates"));
std::string QueryGetStates::_baseSQL;
bool QueryGetStates::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetStates> g_GetStates;

const char*
QueryGetStates::getQueryName() const
{
  return "GETSTATES";
}

void
QueryGetStates::initialize()
{
  if (!_isInitialized)
  {
    QueryGetStatesSQLStatement<QueryGetStates> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSTATES");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetStates::findAllStates(std::vector<tse::State*>& states)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    states.push_back(QueryGetStatesSQLStatement<QueryGetStates>::mapRowToState(row));
  }
  LOG4CXX_INFO(_logger,
               "GETSTATES: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                       << stopCPU() << ")");
  res.freeResult();
} // findAllStates()

///////////////////////////////////////////////////////////
//  QueryGetStatesHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetStatesHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetStatesHistorical"));
bool QueryGetStatesHistorical::_isInitialized = false;
std::string QueryGetStatesHistorical::_baseSQL;
SQLQueryInitializerHelper<QueryGetStatesHistorical> g_GetStatesHistorical;

const char*
QueryGetStatesHistorical::getQueryName() const
{
  return "GETSTATESHISTORICAL";
}

void
QueryGetStatesHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetStatesHistoricalSQLStatement<QueryGetStatesHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSTATESHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetStatesHistorical::findAllStates(std::vector<tse::State*>& states)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    states.push_back(
        QueryGetStatesHistoricalSQLStatement<QueryGetStatesHistorical>::mapRowToState(row));
  }
  LOG4CXX_INFO(_logger,
               "GETSTATESHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                 << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllStatesHistorical()
}
