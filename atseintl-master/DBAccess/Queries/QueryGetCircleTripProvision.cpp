//----------------------------------------------------------------------------
//  File:           QueryGetCircleTripProvision.cpp
//  Description:    QueryGetCircleTripProvision
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

#include "DBAccess/Queries/QueryGetCircleTripProvision.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCircleTripProvisionSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetCircleTripProvision::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetCircleTripProvision"));
std::string QueryGetCircleTripProvision::_baseSQL;
bool QueryGetCircleTripProvision::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCircleTripProvision> g_GetCircleTripProvision;

const char*
QueryGetCircleTripProvision::getQueryName() const
{
  return "GETCIRCLETRIPPROVISION";
}

void
QueryGetCircleTripProvision::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCircleTripProvisionSQLStatement<QueryGetCircleTripProvision> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCIRCLETRIPPROVISION");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetCircleTripProvision::findCircleTripProvision(std::vector<tse::CircleTripProvision*>& lstCTP,
                                                     LocCode& market1,
                                                     LocCode& market2)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, market1);
  substParm(2, market2);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstCTP.push_back(QueryGetCircleTripProvisionSQLStatement<
        QueryGetCircleTripProvision>::mapRowToCircleTripProvision(row));
  }
  LOG4CXX_INFO(_logger,
               "GETCIRCLETRIPPROVISION: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                    << " (" << stopCPU() << ")");
  res.freeResult();
} // findCircleTripProvision()
///////////////////////////////////////////////////////////
//
//  QueryGetCircleTripProvisionHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetCircleTripProvisionHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetCircleTripProvisionHistorical"));
std::string QueryGetCircleTripProvisionHistorical::_baseSQL;
bool QueryGetCircleTripProvisionHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCircleTripProvisionHistorical> g_GetCircleTripProvisionHistorical;

const char*
QueryGetCircleTripProvisionHistorical::getQueryName() const
{
  return "GETCIRCLETRIPPROVISIONHISTORICAL";
}

void
QueryGetCircleTripProvisionHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCircleTripProvisionHistoricalSQLStatement<QueryGetCircleTripProvisionHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCIRCLETRIPPROVISIONHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetCircleTripProvisionHistorical::findCircleTripProvision(
    std::vector<tse::CircleTripProvision*>& lstCTP, LocCode& market1, LocCode& market2)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, market1);
  substParm(2, market2);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstCTP.push_back(QueryGetCircleTripProvisionHistoricalSQLStatement<
        QueryGetCircleTripProvisionHistorical>::mapRowToCircleTripProvision(row));
  }
  LOG4CXX_INFO(_logger,
               "GETCIRCLETRIPPROVISIONHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findCircleTripProvision()

///////////////////////////////////////////////////////////
//
//  QueryGetAllCircleTripProvision
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllCircleTripProvision::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllCircleTripProvision"));
std::string QueryGetAllCircleTripProvision::_baseSQL;
bool QueryGetAllCircleTripProvision::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllCircleTripProvision> g_GetAllCircleTripProvision;

const char*
QueryGetAllCircleTripProvision::getQueryName() const
{
  return "GETALLCIRCLETRIPPROVISION";
}

void
QueryGetAllCircleTripProvision::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllCircleTripProvisionSQLStatement<QueryGetAllCircleTripProvision> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLCIRCLETRIPPROVISION");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllCircleTripProvision::findAllCircleTripProvision(
    std::vector<tse::CircleTripProvision*>& lstCTP)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstCTP.push_back(QueryGetAllCircleTripProvisionSQLStatement<
        QueryGetAllCircleTripProvision>::mapRowToCircleTripProvision(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLCIRCLETRIPPROVISION: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                       << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllCircleTripProvision()
///////////////////////////////////////////////////////////
//
//  QueryGetAllCircleTripProvisionHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllCircleTripProvisionHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllCircleTripProvisionHistorical"));
std::string QueryGetAllCircleTripProvisionHistorical::_baseSQL;
bool QueryGetAllCircleTripProvisionHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllCircleTripProvisionHistorical>
g_GetAllCircleTripProvisionHistorical;

const char*
QueryGetAllCircleTripProvisionHistorical::getQueryName() const
{
  return "GETALLCIRCLETRIPPROVISIONHISTORICAL";
}

void
QueryGetAllCircleTripProvisionHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllCircleTripProvisionHistoricalSQLStatement<QueryGetAllCircleTripProvisionHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLCIRCLETRIPPROVISIONHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllCircleTripProvisionHistorical::findAllCircleTripProvision(
    std::vector<tse::CircleTripProvision*>& lstCTP)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstCTP.push_back(QueryGetAllCircleTripProvisionHistoricalSQLStatement<
        QueryGetAllCircleTripProvisionHistorical>::mapRowToCircleTripProvision(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLCIRCLETRIPPROVISIONHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllCircleTripProvision()
}
