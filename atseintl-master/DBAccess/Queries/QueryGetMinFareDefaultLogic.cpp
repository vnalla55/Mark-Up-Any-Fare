//----------------------------------------------------------------------------
//  File:           QueryGetMinFareDefaultLogic.cpp
//  Description:    QueryGetMinFareDefaultLogic
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
#include "DBAccess/Queries/QueryGetMinFareDefaultLogic.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetMinFareDefaultLogicSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetMinFareDefaultLogic::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetMinFareDefaultLogic"));
std::string QueryGetMinFareDefaultLogic::_baseSQL;
bool QueryGetMinFareDefaultLogic::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMinFareDefaultLogic> g_GetMinFareDefaultLogic;

const char*
QueryGetMinFareDefaultLogic::getQueryName() const
{
  return "GETMINFAREDEFAULTLOGIC";
}

void
QueryGetMinFareDefaultLogic::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMinFareDefaultLogicSQLStatement<QueryGetMinFareDefaultLogic> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMINFAREDEFAULTLOGIC");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMinFareDefaultLogic::findMinFareDefaultLogic(
    std::vector<tse::MinFareDefaultLogic*>& lstMFDL, const CarrierCode& governingCarrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, governingCarrier);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::MinFareDefaultLogic* dl = nullptr;
  tse::MinFareDefaultLogic* dlPrev = nullptr;
  while ((row = res.nextRow()))
  {
    dl = QueryGetMinFareDefaultLogicSQLStatement<
        QueryGetMinFareDefaultLogic>::mapRowToMinFareDefaultLogic(row, dlPrev);
    if (dl != dlPrev)
      lstMFDL.push_back(dl);

    dlPrev = dl;
  }
  LOG4CXX_INFO(_logger,
               "GETMINFAREDEFAULTLOGIC: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                    << " (" << stopCPU() << ") mSecs");
  res.freeResult();
} // findMinFareDefaultLogic()

///////////////////////////////////////////////////////////
//  QueryGetMinFareDefaultLogicHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetMinFareDefaultLogicHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetMinFareDefaultLogicHistorical"));
std::string QueryGetMinFareDefaultLogicHistorical::_baseSQL;
bool QueryGetMinFareDefaultLogicHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMinFareDefaultLogicHistorical> g_GetMinFareDefaultLogicHistorical;

const char*
QueryGetMinFareDefaultLogicHistorical::getQueryName() const
{
  return "GETMINFAREDEFAULTLOGICHISTORICAL";
}

void
QueryGetMinFareDefaultLogicHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMinFareDefaultLogicHistoricalSQLStatement<QueryGetMinFareDefaultLogicHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMINFAREDEFAULTLOGICHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMinFareDefaultLogicHistorical::findMinFareDefaultLogic(
    std::vector<tse::MinFareDefaultLogic*>& lstMFDL,
    const CarrierCode& governingCarrier,
    const VendorCode& vendor,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, governingCarrier);
  substParm(2, vendor);
  substParm(3, startDate);
  substParm(4, endDate);
  substParm(5, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::MinFareDefaultLogic* dl = nullptr;
  tse::MinFareDefaultLogic* dlPrev = nullptr;
  while ((row = res.nextRow()))
  {
    dl = QueryGetMinFareDefaultLogicHistoricalSQLStatement<
        QueryGetMinFareDefaultLogicHistorical>::mapRowToMinFareDefaultLogic(row, dlPrev);
    if (dl != dlPrev)
      lstMFDL.push_back(dl);

    dlPrev = dl;
  }
  LOG4CXX_INFO(_logger,
               "GETMINFAREDEFAULTLOGICHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ") mSecs");
  res.freeResult();
} // findMinFareDefaultLogic()

///////////////////////////////////////////////////////////
//  QueryGetAllMinFareDefaultLogic
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllMinFareDefaultLogic::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllMinFareDefaultLogic"));
std::string QueryGetAllMinFareDefaultLogic::_baseSQL;
bool QueryGetAllMinFareDefaultLogic::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllMinFareDefaultLogic> g_GetAllMinFareDefaultLogic;

const char*
QueryGetAllMinFareDefaultLogic::getQueryName() const
{
  return "GETALLMINFAREDEFAULTLOGIC";
}

void
QueryGetAllMinFareDefaultLogic::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllMinFareDefaultLogicSQLStatement<QueryGetAllMinFareDefaultLogic> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLMINFAREDEFAULTLOGIC");

    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllMinFareDefaultLogic::findAllMinFareDefaultLogic(
    std::vector<tse::MinFareDefaultLogic*>& lstMFDL)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::MinFareDefaultLogic* dl = nullptr;
  tse::MinFareDefaultLogic* dlPrev = nullptr;
  while ((row = res.nextRow()))
  {
    dl = QueryGetAllMinFareDefaultLogicSQLStatement<
        QueryGetAllMinFareDefaultLogic>::mapRowToMinFareDefaultLogic(row, dlPrev);
    if (dl != dlPrev)
      lstMFDL.push_back(dl);

    dlPrev = dl;
  }
  LOG4CXX_INFO(_logger,
               "GETALLMINFAREDEFAULTLOGIC: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                       << " (" << stopCPU() << ") mSecs");
  res.freeResult();
} // findAllMinFareDefaultLogic()

///////////////////////////////////////////////////////////
//  QueryGetAllMinFareDefaultLogiciHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllMinFareDefaultLogicHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllMinFareDefaultLogicHistorical"));
std::string QueryGetAllMinFareDefaultLogicHistorical::_baseSQL;
bool QueryGetAllMinFareDefaultLogicHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllMinFareDefaultLogicHistorical>
g_GetAllMinFareDefaultLogicHistorical;

const char*
QueryGetAllMinFareDefaultLogicHistorical::getQueryName() const
{
  return "GETALLMINFAREDEFAULTLOGICHISTORICAL";
}

void
QueryGetAllMinFareDefaultLogicHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllMinFareDefaultLogicHistoricalSQLStatement<QueryGetAllMinFareDefaultLogicHistorical>
    sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLMINFAREDEFAULTLOGICHISTORICAL");

    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllMinFareDefaultLogicHistorical::findAllMinFareDefaultLogic(
    std::vector<tse::MinFareDefaultLogic*>& lstMFDL)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::MinFareDefaultLogic* dl = nullptr;
  tse::MinFareDefaultLogic* dlPrev = nullptr;
  while ((row = res.nextRow()))
  {
    dl = QueryGetAllMinFareDefaultLogicHistoricalSQLStatement<
        QueryGetAllMinFareDefaultLogicHistorical>::mapRowToMinFareDefaultLogic(row, dlPrev);
    if (dl != dlPrev)
      lstMFDL.push_back(dl);

    dlPrev = dl;
  }
  LOG4CXX_INFO(_logger,
               "GETALLMINFAREDEFAULTLOGICHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ") mSecs");
  res.freeResult();
} // findAllMinFareDefaultLogic()
}
