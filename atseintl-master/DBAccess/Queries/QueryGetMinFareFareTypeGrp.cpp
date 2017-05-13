//----------------------------------------------------------------------------
//  File:           QueryGetMinFareFareTypeGrp.cpp
//  Description:    QueryGetMinFareFareTypeGrp
//  Created:        5/24/2006
// Authors:         Mike Lillis
//
//  Updates:
//
// (C) 2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetMinFareFareTypeGrp.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetMinFareFareTypeGrpSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetMinFareFareTypeGrp::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetMinFareFareTypeGrp"));
std::string QueryGetMinFareFareTypeGrp::_baseSQL;
bool QueryGetMinFareFareTypeGrp::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMinFareFareTypeGrp> g_GetMinFareFareTypeGrp;

const char*
QueryGetMinFareFareTypeGrp::getQueryName() const
{
  return "GETMINFAREFARETYPEGRP";
}

void
QueryGetMinFareFareTypeGrp::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMinFareFareTypeGrpSQLStatement<QueryGetMinFareFareTypeGrp> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMINFAREFARETYPEGRP");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMinFareFareTypeGrp::findMinFareFareTypeGrp(std::vector<tse::MinFareFareTypeGrp*>& lstFTG,
                                                   const std::string& specialProcessName)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, specialProcessName);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::MinFareFareTypeGrp* ftg = nullptr;
  tse::MinFareFareTypeGrp* ftgPrev = nullptr;
  while ((row = res.nextRow()))
  {
    ftg = QueryGetMinFareFareTypeGrpSQLStatement<
        QueryGetMinFareFareTypeGrp>::mapRowToMinFareFareTypeGrp(row, ftgPrev);
    if (ftg != ftgPrev)
      lstFTG.push_back(ftg);

    ftgPrev = ftg;
  }
  LOG4CXX_INFO(_logger,
               "GETMINFAREFARETYPEGRP: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                   << " (" << stopCPU() << ")");
  res.freeResult();
} // findMinFareFareTypeGrp()
///////////////////////////////////////////////////////////
//
//  QueryGetMinFareFareTypeGrpHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetMinFareFareTypeGrpHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetMinFareFareTypeGrpHistorical"));
std::string QueryGetMinFareFareTypeGrpHistorical::_baseSQL;
bool QueryGetMinFareFareTypeGrpHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMinFareFareTypeGrpHistorical> g_GetMinFareFareTypeGrpHistorical;

const char*
QueryGetMinFareFareTypeGrpHistorical::getQueryName() const
{
  return "GETMINFAREFARETYPEGRPHISTORICAL";
}

void
QueryGetMinFareFareTypeGrpHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMinFareFareTypeGrpHistoricalSQLStatement<QueryGetMinFareFareTypeGrpHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMINFAREFARETYPEGRPHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMinFareFareTypeGrpHistorical::findMinFareFareTypeGrp(
    std::vector<tse::MinFareFareTypeGrp*>& lstFTG, const std::string& specialProcessName)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, specialProcessName);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::MinFareFareTypeGrp* ftg = nullptr;
  tse::MinFareFareTypeGrp* ftgPrev = nullptr;
  while ((row = res.nextRow()))
  {
    ftg = QueryGetMinFareFareTypeGrpHistoricalSQLStatement<
        QueryGetMinFareFareTypeGrpHistorical>::mapRowToMinFareFareTypeGrp(row, ftgPrev);
    if (ftg != ftgPrev)
      lstFTG.push_back(ftg);

    ftgPrev = ftg;
  }
  LOG4CXX_INFO(_logger,
               "GETMINFAREFARETYPEGRPHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findMinFareFareTypeGrp()

///////////////////////////////////////////////////////////
//
//  QueryGetAllMinFareFareTypeGrp
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllMinFareFareTypeGrp::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllMinFareFareTypeGrp"));
std::string QueryGetAllMinFareFareTypeGrp::_baseSQL;
bool QueryGetAllMinFareFareTypeGrp::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllMinFareFareTypeGrp> g_GetAllMinFareFareTypeGrp;

const char*
QueryGetAllMinFareFareTypeGrp::getQueryName() const
{
  return "GETALLMINFAREFARETYPEGRP";
}

void
QueryGetAllMinFareFareTypeGrp::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllMinFareFareTypeGrpSQLStatement<QueryGetAllMinFareFareTypeGrp> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLMINFAREFARETYPEGRP");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllMinFareFareTypeGrp::findAllMinFareFareTypeGrp(
    std::vector<tse::MinFareFareTypeGrp*>& lstFTG)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::MinFareFareTypeGrp* ftg = nullptr;
  tse::MinFareFareTypeGrp* ftgPrev = nullptr;
  while ((row = res.nextRow()))
  {
    ftg = QueryGetAllMinFareFareTypeGrpSQLStatement<
        QueryGetAllMinFareFareTypeGrp>::mapRowToMinFareFareTypeGrp(row, ftgPrev);
    if (ftg != ftgPrev)
      lstFTG.push_back(ftg);

    ftgPrev = ftg;
  }
  LOG4CXX_INFO(_logger,
               "GETALLMINFAREFARETYPEGRP: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllMinFareFareTypeGrp()
///////////////////////////////////////////////////////////
//
//  QueryGetAllMinFareFareTypeGrpHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllMinFareFareTypeGrpHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllMinFareFareTypeGrpHistorical"));
std::string QueryGetAllMinFareFareTypeGrpHistorical::_baseSQL;
bool QueryGetAllMinFareFareTypeGrpHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllMinFareFareTypeGrpHistorical>
g_GetAllMinFareFareTypeGrpHistorical;

const char*
QueryGetAllMinFareFareTypeGrpHistorical::getQueryName() const
{
  return "GETALLMINFAREFARETYPEGRPHISTORICAL";
}

void
QueryGetAllMinFareFareTypeGrpHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllMinFareFareTypeGrpHistoricalSQLStatement<QueryGetAllMinFareFareTypeGrpHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLMINFAREFARETYPEGRPHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllMinFareFareTypeGrpHistorical::findAllMinFareFareTypeGrp(
    std::vector<tse::MinFareFareTypeGrp*>& lstFTG)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::MinFareFareTypeGrp* ftg = nullptr;
  tse::MinFareFareTypeGrp* ftgPrev = nullptr;
  while ((row = res.nextRow()))
  {
    ftg = QueryGetAllMinFareFareTypeGrpHistoricalSQLStatement<
        QueryGetAllMinFareFareTypeGrpHistorical>::mapRowToMinFareFareTypeGrp(row, ftgPrev);
    if (ftg != ftgPrev)
      lstFTG.push_back(ftg);

    ftgPrev = ftg;
  }
  LOG4CXX_INFO(_logger,
               "GETALLMINFAREFARETYPEGRPHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllMinFareFareTypeGrpHistorical()
}
