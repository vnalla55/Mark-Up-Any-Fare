//----------------------------------------------------------------------------
//  File:           QueryGetTable987.cpp
//  Description:    QueryGetTable987
//  Created:        4/24/2007
// Authors:         Grzegorz Cholewiak
//
//  Updates:
//
// � 2007, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetTable987.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetTable987SQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetTable987::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTable987"));
std::string QueryGetTable987::_baseSQL;
bool QueryGetTable987::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTable987> g_GetTable987;

const char*
QueryGetTable987::getQueryName() const
{
  return "GETTABLE987";
}

void
QueryGetTable987::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTable987SQLStatement<QueryGetTable987> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTABLE987");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTable987::findWaiver(std::vector<tse::Waiver*>& table987s,
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
    table987s.push_back(QueryGetTable987SQLStatement<QueryGetTable987>::mapRowToWaiver(row));
  }
  LOG4CXX_INFO(_logger,
               "GETTABLE987: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                         << stopCPU() << ")");
  res.freeResult();
} // findWaiver()

///////////////////////////////////////////////////////////
//  QueryGetTable987Historical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetTable987Historical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetTable987Historical"));
std::string QueryGetTable987Historical::_baseSQL;
bool QueryGetTable987Historical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTable987Historical> g_GetTable987Historical;

const char*
QueryGetTable987Historical::getQueryName() const
{
  return "GETTABLE987HISTORICAL";
}

void
QueryGetTable987Historical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTable987HistoricalSQLStatement<QueryGetTable987Historical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTABLE987HISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTable987Historical::findWaiver(std::vector<tse::Waiver*>& table987s,
                                       const VendorCode& vendor,
                                       int itemNo)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char itemStr[15];
  sprintf(itemStr, "%d", itemNo);

  substParm(vendor, 1);
  substParm(itemStr, 2);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    table987s.push_back(
        QueryGetTable987HistoricalSQLStatement<QueryGetTable987Historical>::mapRowToWaiver(row));
  }
  LOG4CXX_INFO(_logger,
               "GETTABLE987HISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                   << " (" << stopCPU() << ")");
  res.freeResult();
} // findWaiver()

///////////////////////////////////////////////////////////
//  QueryGetAllTable987Historical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllTable987Historical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetAllTable987Historical"));
std::string QueryGetAllTable987Historical::_baseSQL;
bool QueryGetAllTable987Historical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllTable987Historical> g_GetAllTable987Historical;

const char*
QueryGetAllTable987Historical::getQueryName() const
{
  return "GETALLTABLE987HISTORICAL";
}

void
QueryGetAllTable987Historical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllTable987HistoricalSQLStatement<QueryGetAllTable987Historical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLTABLE987HISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllTable987Historical::findAllWaivers(std::vector<tse::Waiver*>& table987s)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    table987s.push_back(
        QueryGetAllTable987HistoricalSQLStatement<QueryGetAllTable987Historical>::mapRowToWaiver(
            row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLTABLE987HISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllWaivers()
}
