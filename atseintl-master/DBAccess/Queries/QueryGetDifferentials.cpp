//----------------------------------------------------------------------------
//  File:           QueryGetDifferentials.cpp
//  Description:    QueryGetDifferentials
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
#include "DBAccess/Queries/QueryGetDifferentials.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetDifferentialsSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetDifferentials::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetDifferentials"));
std::string QueryGetDifferentials::_baseSQL;
bool QueryGetDifferentials::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetDifferentials> g_GetDifferentials;

const char*
QueryGetDifferentials::getQueryName() const
{
  return "GETDIFFERENTIALS";
}

void
QueryGetDifferentials::initialize()
{
  if (!_isInitialized)
  {
    QueryGetDifferentialsSQLStatement<QueryGetDifferentials> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETDIFFERENTIALS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetDifferentials::findDifferentials(std::vector<tse::Differentials*>& diffs,
                                         const CarrierCode& cxr)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, cxr);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    diffs.push_back(
        QueryGetDifferentialsSQLStatement<QueryGetDifferentials>::mapRowToDifferentials(row));
  }
  LOG4CXX_INFO(_logger,
               "GETDIFFERENTIALS: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                              << stopCPU() << ")");
  res.freeResult();
} // findDifferentials()

///////////////////////////////////////////////////////////
//  QueryGetDifferentialsHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetDifferentialsHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetDifferentialsHistorical"));
std::string QueryGetDifferentialsHistorical::_baseSQL;
bool QueryGetDifferentialsHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetDifferentialsHistorical> g_GetDifferentialsHistorical;

const char*
QueryGetDifferentialsHistorical::getQueryName() const
{
  return "GETDIFFERENTIALSHISTORICAL";
}

void
QueryGetDifferentialsHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetDifferentialsHistoricalSQLStatement<QueryGetDifferentialsHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETDIFFERENTIALSHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetDifferentialsHistorical::findDifferentials(std::vector<tse::Differentials*>& diffs,
                                                   const CarrierCode& cxr)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, cxr);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    diffs.push_back(QueryGetDifferentialsHistoricalSQLStatement<
        QueryGetDifferentialsHistorical>::mapRowToDifferentials(row));
  }
  LOG4CXX_INFO(_logger,
               "GETDIFFERENTIALSHISTORICAL: NumRows = " << res.numRows() << " Time = "
                                                        << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findDifferentials()

///////////////////////////////////////////////////////////
//  QueryGetAllDifferentials
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllDifferentials::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllDifferentials"));
std::string QueryGetAllDifferentials::_baseSQL;
bool QueryGetAllDifferentials::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllDifferentials> g_GetAllDifferentials;

const char*
QueryGetAllDifferentials::getQueryName() const
{
  return "GETALLDIFFERENTIALS";
}

void
QueryGetAllDifferentials::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllDifferentialsSQLStatement<QueryGetAllDifferentials> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLDIFFERENTIALS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllDifferentials::findAllDifferentials(std::vector<tse::Differentials*>& diffs)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    diffs.push_back(
        QueryGetAllDifferentialsSQLStatement<QueryGetAllDifferentials>::mapRowToDifferentials(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLDIFFERENTIALS: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                 << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllDifferentials()

///////////////////////////////////////////////////////////
//  QueryGetAllDifferentialsHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllDifferentialsHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllDifferentialsHistorical"));
std::string QueryGetAllDifferentialsHistorical::_baseSQL;
bool QueryGetAllDifferentialsHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllDifferentialsHistorical> g_GetAllDifferentialsHistorical;

const char*
QueryGetAllDifferentialsHistorical::getQueryName() const
{
  return "GETALLDIFFERENTIALSHISTORICAL";
}

void
QueryGetAllDifferentialsHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllDifferentialsHistoricalSQLStatement<QueryGetAllDifferentialsHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLDIFFERENTIALSHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllDifferentialsHistorical::findAllDifferentials(std::vector<tse::Differentials*>& diffs)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    diffs.push_back(QueryGetAllDifferentialsHistoricalSQLStatement<
        QueryGetAllDifferentialsHistorical>::mapRowToDifferentials(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLDIFFERENTIALSHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllDifferentials()
}
