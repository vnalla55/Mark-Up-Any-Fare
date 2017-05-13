//----------------------------------------------------------------------------
//  (C) 2009, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetSeasonalityDOW.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetSeasonalityDOWSQLStatement.h"

namespace tse
{

log4cxx::LoggerPtr
QueryGetSeasonalityDOW::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetSeasonalityDOW"));
std::string QueryGetSeasonalityDOW::_baseSQL;
bool QueryGetSeasonalityDOW::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSeasonalityDOW> g_QueryGetSeasonalityDOW;

const char*
QueryGetSeasonalityDOW::getQueryName() const
{
  return "QUERYGETSEASONALITYDOW";
}

void
QueryGetSeasonalityDOW::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSeasonalityDOWSQLStatement<QueryGetSeasonalityDOW> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("QUERYGETSEASONALITYDOW");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetSeasonalityDOW::findSeasonalityDOW(std::vector<SeasonalityDOW*>& SeasonalityDOWVec,
                                           const VendorCode& vendor,
                                           int itemNo)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char itemStr[15];
  sprintf(itemStr, "%d", itemNo);

  substParm(vendor, 1);
  substParm(itemStr, 2);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    SeasonalityDOWVec.push_back(
        QueryGetSeasonalityDOWSQLStatement<QueryGetSeasonalityDOW>::mapRowToSeasonalityDOW(row));
  }
  LOG4CXX_INFO(_logger,
               "GETSEASONALITYDOW: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                               << stopCPU() << ")");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetAllSeasonalityDOW::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetAllSeasonalityDOW"));
std::string QueryGetAllSeasonalityDOW::_baseSQL;
bool QueryGetAllSeasonalityDOW::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllSeasonalityDOW> g_QueryGetAllSeasonalityDOW;

const char*
QueryGetAllSeasonalityDOW::getQueryName() const
{
  return "QUERYGETALLSEASONALITYDOW";
}

void
QueryGetAllSeasonalityDOW::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllSeasonalityDOWSQLStatement<QueryGetAllSeasonalityDOW> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("QUERYGETALLSEASONALITYDOW");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetAllSeasonalityDOW::findAllSeasonalityDOW(std::vector<SeasonalityDOW*>& seasonalityDOWVec)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    seasonalityDOWVec.push_back(
        QueryGetAllSeasonalityDOWSQLStatement<QueryGetAllSeasonalityDOW>::mapRowToSeasonalityDOW(
            row));
  }
  LOG4CXX_INFO(_logger,
               "QUERYGETALLSEASONALITYDOW: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                       << " (" << stopCPU() << ")");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetSeasonalityDOWHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetSeasonalityDOWHistorical"));
std::string QueryGetSeasonalityDOWHistorical::_baseSQL;
bool QueryGetSeasonalityDOWHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSeasonalityDOWHistorical> g_QueryGetSeasonalityDOWHistorical;

const char*
QueryGetSeasonalityDOWHistorical::getQueryName() const
{
  return "QUERYGETSEASONALITYDOWHISTORICAL";
}

void
QueryGetSeasonalityDOWHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSeasonalityDOWHistoricalSQLStatement<QueryGetSeasonalityDOWHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("QUERYGETSEASONALITYDOWHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetSeasonalityDOWHistorical::findSeasonalityDOW(
    std::vector<SeasonalityDOW*>& SeasonalityDOWVec, const VendorCode& vendor, int itemNo)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char itemStr[15];
  sprintf(itemStr, "%d", itemNo);

  substParm(vendor, 1);
  substParm(itemStr, 2);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    SeasonalityDOWVec.push_back(QueryGetSeasonalityDOWHistoricalSQLStatement<
        QueryGetSeasonalityDOWHistorical>::mapRowToSeasonalityDOW(row));
  }
  LOG4CXX_INFO(_logger,
               "QUERYGETSEASONALITYDOWHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
}
}
