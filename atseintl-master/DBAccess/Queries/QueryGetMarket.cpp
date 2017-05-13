//----------------------------------------------------------------------------
//  File:           QueryGetMarket.cpp
//  Description:    QueryGetMarket
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
#include "DBAccess/Queries/QueryGetMarket.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetMarketSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetMkt::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetMkt"));
std::string QueryGetMkt::_baseSQL;
bool QueryGetMkt::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMkt> g_GetMkt;

const char*
QueryGetMkt::getQueryName() const
{
  return "GETMKT";
}

void
QueryGetMkt::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMktSQLStatement<QueryGetMkt> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMKT");

    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMkt::findLoc(std::vector<tse::Loc*>& locs, const LocCode& loc)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, loc);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    locs.push_back(QueryGetMktSQLStatement<QueryGetMkt>::mapRowToLoc(row));
  }
  LOG4CXX_INFO(_logger,
               "GETMKT: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                    << stopCPU() << ")");
  res.freeResult();
} // findLoc()

///////////////////////////////////////////////////////////
//  QueryGetMktHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetMktHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetMktHistorical"));
std::string QueryGetMktHistorical::_baseSQL;
bool QueryGetMktHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMktHistorical> g_GetMktHistorical;

const char*
QueryGetMktHistorical::getQueryName() const
{
  return "GETMKTHISTORICAL";
}

void
QueryGetMktHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMktHistoricalSQLStatement<QueryGetMktHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMKTHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMktHistorical::findLoc(std::vector<tse::Loc*>& locs,
                               const LocCode& loc,
                               const DateTime& startDate,
                               const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, loc);
  substParm(2, startDate);
  substParm(3, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    locs.push_back(QueryGetMktHistoricalSQLStatement<QueryGetMktHistorical>::mapRowToLoc(row));
  }
  LOG4CXX_INFO(_logger,
               "GETMKTHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                              << stopCPU() << ")");
  res.freeResult();
} // findLoc()

///////////////////////////////////////////////////////////
//  QueryGetAllMkt
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllMkt::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllMkt"));
std::string QueryGetAllMkt::_baseSQL;
bool QueryGetAllMkt::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllMkt> g_GetAllMkt;

const char*
QueryGetAllMkt::getQueryName() const
{
  return "GETALLMKT";
}

void
QueryGetAllMkt::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllMktSQLStatement<QueryGetAllMkt> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLMKT");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllMkt::findAllLoc(std::vector<tse::Loc*>& locs)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    locs.push_back(QueryGetAllMktSQLStatement<QueryGetAllMkt>::mapRowToLoc(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLMKT: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                       << stopCPU() << ")");
  res.freeResult();
} // findAllLoc()
}
