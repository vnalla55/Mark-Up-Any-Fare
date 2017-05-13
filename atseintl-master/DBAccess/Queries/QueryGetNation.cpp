//----------------------------------------------------------------------------
//  File:           QueryGetNation.cpp
//  Description:    QueryGetNation
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

#include "DBAccess/Queries/QueryGetNation.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetNationSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetNation::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetNation"));
std::string QueryGetNation::_baseSQL;
bool QueryGetNation::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetNation> g_GetNation;

const char*
QueryGetNation::getQueryName() const
{
  return "GETNATION";
}

void
QueryGetNation::initialize()
{
  if (!_isInitialized)
  {
    QueryGetNationSQLStatement<QueryGetNation> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETNATION");

    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetNation::findNation(std::vector<tse::Nation*>& nations, const NationCode& nation)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, nation);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    nations.push_back(QueryGetNationSQLStatement<QueryGetNation>::mapRowToNation(row));
  }
  LOG4CXX_INFO(_logger,
               "GETNATION: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                       << stopCPU() << ")");
  res.freeResult();
} // findNation()

///////////////////////////////////////////////////////////
//
//  QueryGetNations
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetNations::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetNations"));
std::string QueryGetNations::_baseSQL;
bool QueryGetNations::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetNations> g_GetNations;

const char*
QueryGetNations::getQueryName() const
{
  return "GETNATIONS";
}

void
QueryGetNations::initialize()
{
  if (!_isInitialized)
  {
    QueryGetNationsSQLStatement<QueryGetNations> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETNATIONS");

    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetNations::findAllNations(std::vector<tse::Nation*>& nations)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    nations.push_back(QueryGetNationsSQLStatement<QueryGetNations>::mapRowToNation(row));
  }
  LOG4CXX_INFO(_logger,
               "GETNATIONS: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                        << stopCPU() << ")");
  res.freeResult();
} // findAllNations()

///////////////////////////////////////////////////////////
//
//  QueryGetNationHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetNationHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetNationHistorical"));
bool QueryGetNationHistorical::_isInitialized = false;
std::string QueryGetNationHistorical::_baseSQL;
SQLQueryInitializerHelper<QueryGetNationHistorical> g_GetNationHistorical;

const char*
QueryGetNationHistorical::getQueryName() const
{
  return "GETNATIONHISTORICAL";
}

void
QueryGetNationHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetNationHistoricalSQLStatement<QueryGetNationHistorical> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETNATIONHISTORICAL");

    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetNationHistorical::findNation(std::vector<tse::Nation*>& nations, const NationCode& nation)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, nation);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    nations.push_back(
        QueryGetNationHistoricalSQLStatement<QueryGetNationHistorical>::mapRowToNation(row));
  }
  LOG4CXX_INFO(_logger,
               "GETNATIONHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                 << " (" << stopCPU() << ")");
  res.freeResult();
} // findNationHistorical()

///////////////////////////////////////////////////////////
//
//  QueryGetNationsHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetNationsHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetNationsHistorical"));
bool QueryGetNationsHistorical::_isInitialized = false;
std::string QueryGetNationsHistorical::_baseSQL;
SQLQueryInitializerHelper<QueryGetNationsHistorical> g_GetNationsHistorical;

const char*
QueryGetNationsHistorical::getQueryName() const
{
  return "GETNATIONSHISTORICAL";
}

void
QueryGetNationsHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetNationsHistoricalSQLStatement<QueryGetNationsHistorical> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETNATIONSHISTORICAL");

    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetNationsHistorical::findAllNations(std::vector<tse::Nation*>& nations)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    nations.push_back(
        QueryGetNationsHistoricalSQLStatement<QueryGetNationsHistorical>::mapRowToNation(row));
  }
  LOG4CXX_INFO(_logger,
               "GETNATIONSHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                  << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllNationsHistorical()
}
