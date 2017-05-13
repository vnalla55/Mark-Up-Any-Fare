//----------------------------------------------------------------------------
//  File:           QueryGetCabin.cpp
//  Description:    QueryGetCabin
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

#include "DBAccess/Queries/QueryGetCabin.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCabinSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetCabin::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetCabin"));
std::string QueryGetCabin::_baseSQL;
bool QueryGetCabin::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCabin> g_GetCabin;

const char*
QueryGetCabin::getQueryName() const
{
  return "GETCABIN";
}

void
QueryGetCabin::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCabinSQLStatement<QueryGetCabin> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCABIN");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetCabin::findCabin(std::vector<tse::Cabin*>& cabins, const CarrierCode& carrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(carrier, 1);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    cabins.push_back(QueryGetCabinSQLStatement<QueryGetCabin>::mapRowToCabin(row));
  }
  LOG4CXX_INFO(_logger,
               "GETCABIN: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                      << stopCPU() << ")");
  res.freeResult();
} // findCabin()

///////////////////////////////////////////////////////////
//
//  QueryGetCabinHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetCabinHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetCabinHistorical"));
std::string QueryGetCabinHistorical::_baseSQL;
bool QueryGetCabinHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCabinHistorical> g_GetCabinHistorical;

const char*
QueryGetCabinHistorical::getQueryName() const
{
  return "GETCABINHISTORICAL";
}

void
QueryGetCabinHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCabinHistoricalSQLStatement<QueryGetCabinHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCABINHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetCabinHistorical::findCabin(std::vector<tse::Cabin*>& cabins, const CarrierCode& carrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(carrier, 1);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    cabins.push_back(
        QueryGetCabinHistoricalSQLStatement<QueryGetCabinHistorical>::mapRowToCabin(row));
  }
  LOG4CXX_INFO(_logger,
               "GETCABINHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                << " (" << stopCPU() << ")");
  res.freeResult();
} // findCabin()

///////////////////////////////////////////////////////////
//
//  QueryGetAllCabin
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllCabin::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllCabin"));
std::string QueryGetAllCabin::_baseSQL;
bool QueryGetAllCabin::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllCabin> g_GetAllCabin;

const char*
QueryGetAllCabin::getQueryName() const
{
  return "GETALLCABIN";
}

void
QueryGetAllCabin::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllCabinSQLStatement<QueryGetAllCabin> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLCABIN");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllCabin::findAllCabin(std::vector<tse::Cabin*>& cabins)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    cabins.push_back(QueryGetAllCabinSQLStatement<QueryGetAllCabin>::mapRowToCabin(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLCABIN: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                         << stopCPU() << ")");
  res.freeResult();
} // findAllCabin()

///////////////////////////////////////////////////////////
//
//  QueryGetAllCabinHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllCabinHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllCabinHistorical"));
std::string QueryGetAllCabinHistorical::_baseSQL;
bool QueryGetAllCabinHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllCabinHistorical> g_GetAllCabinHistorical;

const char*
QueryGetAllCabinHistorical::getQueryName() const
{
  return "GETALLCABINHISTORICAL";
}

void
QueryGetAllCabinHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllCabinHistoricalSQLStatement<QueryGetAllCabinHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLCABINHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllCabinHistorical::findAllCabin(std::vector<tse::Cabin*>& cabins)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    cabins.push_back(
        QueryGetAllCabinHistoricalSQLStatement<QueryGetAllCabinHistorical>::mapRowToCabin(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLCABINHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                   << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllCabin()
}
