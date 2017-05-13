//----------------------------------------------------------------------------
//  File:           QueryGetSurfaceSectorExempt.cpp
//  Description:    QueryGetSurfaceSectorExempt
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

#include "DBAccess/Queries/QueryGetSurfaceSectorExempt.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetSurfaceSectorExemptSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetSurfaceSectorExempt::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetSurfaceSectorExempt"));
std::string QueryGetSurfaceSectorExempt::_baseSQL;
bool QueryGetSurfaceSectorExempt::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSurfaceSectorExempt> g_GetSurfaceSectorExempt;

const char*
QueryGetSurfaceSectorExempt::getQueryName() const
{
  return "GETSURFACESECTOREXEMPT";
}

void
QueryGetSurfaceSectorExempt::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSurfaceSectorExemptSQLStatement<QueryGetSurfaceSectorExempt> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSURFACESECTOREXEMPT");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetSurfaceSectorExempt::findSurfaceSectorExempt(std::vector<tse::SurfaceSectorExempt*>& lstSSE,
                                                     const LocCode& origLoc,
                                                     const LocCode& destLoc)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, origLoc);
  substParm(2, destLoc);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstSSE.push_back(QueryGetSurfaceSectorExemptSQLStatement<
        QueryGetSurfaceSectorExempt>::mapRowToSurfaceSectorExempt(row));
  }
  LOG4CXX_INFO(_logger,
               "GETSURFACESECTOREXEMPT: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                    << " (" << stopCPU() << ")");
  res.freeResult();
} // findSurfaceSectorExempt()

///////////////////////////////////////////////////////////
//
//  QueryGetSurfaceSectorExemptHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetSurfaceSectorExemptHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetSurfaceSectorExemptHistorical"));
std::string QueryGetSurfaceSectorExemptHistorical::_baseSQL;
bool QueryGetSurfaceSectorExemptHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSurfaceSectorExemptHistorical> g_GetSurfaceSectorExemptHistorical;

const char*
QueryGetSurfaceSectorExemptHistorical::getQueryName() const
{
  return "GETSURFACESECTOREXEMPTHISTORICAL";
}

void
QueryGetSurfaceSectorExemptHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSurfaceSectorExemptHistoricalSQLStatement<QueryGetSurfaceSectorExemptHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSURFACESECTOREXEMPTHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetSurfaceSectorExemptHistorical::findSurfaceSectorExempt(
    std::vector<tse::SurfaceSectorExempt*>& lstSSE, const LocCode& origLoc, const LocCode& destLoc)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, origLoc);
  substParm(2, destLoc);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstSSE.push_back(QueryGetSurfaceSectorExemptHistoricalSQLStatement<
        QueryGetSurfaceSectorExemptHistorical>::mapRowToSurfaceSectorExempt(row));
  }
  LOG4CXX_INFO(_logger,
               "GETSURFACESECTOREXEMPTHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findSurfaceSectorExemptHistorical()

///////////////////////////////////////////////////////////
//
//  QueryGetAllSurfaceSectorExempt
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllSurfaceSectorExempt::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllSurfaceSectorExempt"));
std::string QueryGetAllSurfaceSectorExempt::_baseSQL;
bool QueryGetAllSurfaceSectorExempt::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllSurfaceSectorExempt> g_GetAllSurfaceSectorExempt;

const char*
QueryGetAllSurfaceSectorExempt::getQueryName() const
{
  return "GETALLSURFACESECTOREXEMPT";
}

void
QueryGetAllSurfaceSectorExempt::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllSurfaceSectorExemptSQLStatement<QueryGetAllSurfaceSectorExempt> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLSURFACESECTOREXEMPT");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllSurfaceSectorExempt::findAllSurfaceSectorExempt(
    std::vector<tse::SurfaceSectorExempt*>& lstSSE)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstSSE.push_back(QueryGetAllSurfaceSectorExemptSQLStatement<
        QueryGetAllSurfaceSectorExempt>::mapRowToSurfaceSectorExempt(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLSURFACESECTOREXEMPT: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                       << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllSurfaceSectorExempt()

///////////////////////////////////////////////////////////
//
//  QueryGetAllSurfaceSectorExemptHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllSurfaceSectorExemptHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllSurfaceSectorExemptHistorical"));
std::string QueryGetAllSurfaceSectorExemptHistorical::_baseSQL;
bool QueryGetAllSurfaceSectorExemptHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllSurfaceSectorExemptHistorical>
g_GetAllSurfaceSectorExemptHistorical;

const char*
QueryGetAllSurfaceSectorExemptHistorical::getQueryName() const
{
  return "GETALLSURFACESECTOREXEMPTHISTORICAL";
}

void
QueryGetAllSurfaceSectorExemptHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllSurfaceSectorExemptHistoricalSQLStatement<QueryGetAllSurfaceSectorExemptHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLSURFACESECTOREXEMPTHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllSurfaceSectorExemptHistorical::findAllSurfaceSectorExempt(
    std::vector<tse::SurfaceSectorExempt*>& lstSSE)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstSSE.push_back(QueryGetAllSurfaceSectorExemptHistoricalSQLStatement<
        QueryGetAllSurfaceSectorExemptHistorical>::mapRowToSurfaceSectorExempt(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLSURFACESECTOREXEMPTHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllSurfaceSectorExemptHistorical()
}
