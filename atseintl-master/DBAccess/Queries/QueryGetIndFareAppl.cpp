//----------------------------------------------------------------------------
//  File:           QueryGetIndFareAppl.cpp
//  Description:    QueryGetIndFareAppl
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
#include "DBAccess/Queries/QueryGetIndFareAppl.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetIndFareApplSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetIndFareAppl::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetIndFareAppl"));
std::string QueryGetIndFareAppl::_baseSQL;
bool QueryGetIndFareAppl::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetIndFareAppl> g_GetIndFareAppl;

const char*
QueryGetIndFareAppl::getQueryName() const
{
  return "GETINDFAREAPPL";
}

void
QueryGetIndFareAppl::initialize()
{
  if (!_isInitialized)
  {
    QueryGetIndFareApplSQLStatement<QueryGetIndFareAppl> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETINDFAREAPPL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetIndFareAppl::findIndustryFareAppl(std::vector<tse::IndustryFareAppl*>& indFares,
                                          const Indicator& selectionType,
                                          const CarrierCode& carrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, selectionType);
  substParm(2, carrier);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  tse::IndustryFareAppl* prevIndFare = nullptr;
  DateTime prevDate;
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    tse::IndustryFareAppl* indFare =
        QueryGetIndFareApplSQLStatement<QueryGetIndFareAppl>::mapRowToIndustryFareAppl(
            row, prevIndFare, prevDate);
    if (indFare != prevIndFare)
    {
      indFares.push_back(indFare);
      prevIndFare = indFare;
    }
  }
  LOG4CXX_INFO(_logger,
               "GETINDFAREAPPL: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                            << stopCPU() << ")");
  res.freeResult();
} // findIndustryFareAppl()

///////////////////////////////////////////////////////////
//  QueryGetIndFareApplHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetIndFareApplHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetIndFareApplHistorical"));
std::string QueryGetIndFareApplHistorical::_baseSQL;
bool QueryGetIndFareApplHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetIndFareApplHistorical> g_GetIndFareApplHistorical;

const char*
QueryGetIndFareApplHistorical::getQueryName() const
{
  return "GETINDFAREAPPLHISTORICAL";
}

void
QueryGetIndFareApplHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetIndFareApplHistoricalSQLStatement<QueryGetIndFareApplHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETINDFAREAPPLHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetIndFareApplHistorical::findIndustryFareAppl(std::vector<tse::IndustryFareAppl*>& indFares,
                                                    const Indicator& selectionType,
                                                    const CarrierCode& carrier,
                                                    const DateTime& startDate,
                                                    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, selectionType);
  substParm(2, carrier);
  substParm(3, startDate);
  substParm(4, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  tse::IndustryFareAppl* prevIndFare = nullptr;
  DateTime prevDate;
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    tse::IndustryFareAppl* indFare = QueryGetIndFareApplHistoricalSQLStatement<
        QueryGetIndFareApplHistorical>::mapRowToIndustryFareAppl(row, prevIndFare, prevDate);
    if (indFare != prevIndFare)
    {
      indFares.push_back(indFare);
      prevIndFare = indFare;
    }
  }
  LOG4CXX_INFO(_logger,
               "GETINDFAREAPPLHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ")");
  res.freeResult();
} // findIndustryFareAppl()

///////////////////////////////////////////////////////////
//  QueryGetAllIndFareAppl
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllIndFareAppl::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllIndFareAppl"));
std::string QueryGetAllIndFareAppl::_baseSQL;
bool QueryGetAllIndFareAppl::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllIndFareAppl> g_GetAllIndFareAppl;

const char*
QueryGetAllIndFareAppl::getQueryName() const
{
  return "GETALLINDFAREAPPL";
}

void
QueryGetAllIndFareAppl::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllIndFareApplSQLStatement<QueryGetAllIndFareAppl> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLINDFAREAPPL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllIndFareAppl::findAllIndustryFareAppl(std::vector<tse::IndustryFareAppl*>& indFares)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  tse::IndustryFareAppl* prevIndFare = nullptr;
  DateTime prevDate;
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    tse::IndustryFareAppl* indFare =
        QueryGetAllIndFareApplSQLStatement<QueryGetAllIndFareAppl>::mapRowToIndustryFareAppl(
            row, prevIndFare, prevDate);
    if (indFare != prevIndFare)
    {
      indFares.push_back(indFare);
      prevIndFare = indFare;
    }
  }
  LOG4CXX_INFO(_logger,
               "GETALLINDFAREAPPL: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                               << stopCPU() << ")");
  res.freeResult();
} // findAllIndustryFareAppl()

///////////////////////////////////////////////////////////
//  QueryGetAllIndFareApplHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllIndFareApplHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllIndFareApplHistorical"));
std::string QueryGetAllIndFareApplHistorical::_baseSQL;
bool QueryGetAllIndFareApplHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllIndFareApplHistorical> g_GetAllIndFareApplHistorical;

const char*
QueryGetAllIndFareApplHistorical::getQueryName() const
{
  return "GETALLINDFAREAPPLHISTORICAL";
}

void
QueryGetAllIndFareApplHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllIndFareApplHistoricalSQLStatement<QueryGetAllIndFareApplHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLINDFAREAPPLHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllIndFareApplHistorical::findAllIndustryFareAppl(
    std::vector<tse::IndustryFareAppl*>& indFares)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  tse::IndustryFareAppl* prevIndFare = nullptr;
  DateTime prevDate;
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    tse::IndustryFareAppl* indFare = QueryGetAllIndFareApplHistoricalSQLStatement<
        QueryGetAllIndFareApplHistorical>::mapRowToIndustryFareAppl(row, prevIndFare, prevDate);
    if (indFare != prevIndFare)
    {
      indFares.push_back(indFare);
      prevIndFare = indFare;
    }
  }
  LOG4CXX_INFO(_logger,
               "GETALLINDFAREAPPLHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllIndustryFareAppl()
}
