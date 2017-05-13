//----------------------------------------------------------------------------
//  File:           QueryGetPfcMultiAirport.cpp
//  Description:    QueryGetPfcMultiAirport
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

#include "DBAccess/Queries/QueryGetPfcMultiAirport.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetPfcMultiAirportSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetPfcMultiAirport::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetPfcMultiAirport"));
std::string QueryGetPfcMultiAirport::_baseSQL;
bool QueryGetPfcMultiAirport::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetPfcMultiAirport> g_GetPfcMultiAirport;

const char*
QueryGetPfcMultiAirport::getQueryName() const
{
  return "GETPFCMULTIAIRPORT";
}

void
QueryGetPfcMultiAirport::initialize()
{
  if (!_isInitialized)
  {
    QueryGetPfcMultiAirportSQLStatement<QueryGetPfcMultiAirport> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETPFCMULTIAIRPORT");
    substTableDef(&_baseSQL);

    _isInitialized = true;
  }
} // initialize()

void
QueryGetPfcMultiAirport::findPfcMultiAirport(std::vector<tse::PfcMultiAirport*>& lstMA,
                                             const LocCode& loc)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, loc);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::PfcMultiAirport* ma = nullptr;
  tse::PfcMultiAirport* maPrev = nullptr;
  while ((row = res.nextRow()))
  {
    ma = QueryGetPfcMultiAirportSQLStatement<QueryGetPfcMultiAirport>::mapRowToPfcMultiAirport(
        row, maPrev);
    if (ma != maPrev)
      lstMA.push_back(ma);

    maPrev = ma;
  }
  LOG4CXX_INFO(_logger,
               "GETPFCMULTIAIRPORT: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                << " (" << stopCPU() << ")");
  res.freeResult();
} // findPfcMultiAirport()

///////////////////////////////////////////////////////////
//
//  QueryGetAllPfcMultiAirport
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllPfcMultiAirport::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllPfcMultiAirport"));
std::string QueryGetAllPfcMultiAirport::_baseSQL;
bool QueryGetAllPfcMultiAirport::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllPfcMultiAirport> g_GetAllPfcMultiAirport;

const char*
QueryGetAllPfcMultiAirport::getQueryName() const
{
  return "GETALLPFCMULTIAIRPORT";
}

void
QueryGetAllPfcMultiAirport::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllPfcMultiAirportSQLStatement<QueryGetAllPfcMultiAirport> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLPFCMULTIAIRPORT");
    substTableDef(&_baseSQL);

    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllPfcMultiAirport::findAllPfcMultiAirport(std::vector<tse::PfcMultiAirport*>& lstMA)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::PfcMultiAirport* ma = nullptr;
  tse::PfcMultiAirport* maPrev = nullptr;
  while ((row = res.nextRow()))
  {
    ma =
        QueryGetAllPfcMultiAirportSQLStatement<QueryGetAllPfcMultiAirport>::mapRowToPfcMultiAirport(
            row, maPrev);
    if (ma != maPrev)
      lstMA.push_back(ma);

    maPrev = ma;
  }
  LOG4CXX_INFO(_logger,
               "GETALLPFCMULTIAIRPORT: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                   << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllPfcMultiAirport()

///////////////////////////////////////////////////////////
//
//  QueryGetPfcMultiAirportHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetPfcMultiAirportHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetPfcMultiAirportHistorical"));
std::string QueryGetPfcMultiAirportHistorical::_baseSQL;
bool QueryGetPfcMultiAirportHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetPfcMultiAirportHistorical> g_GetPfcMultiAirportHistorical;

const char*
QueryGetPfcMultiAirportHistorical::getQueryName() const
{
  return "GETPFCMULTIAIRPORTHISTORICAL";
}

void
QueryGetPfcMultiAirportHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetPfcMultiAirportHistoricalSQLStatement<QueryGetPfcMultiAirportHistorical> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETPFCMULTIAIRPORTHISTORICAL");
    substTableDef(&_baseSQL);

    _isInitialized = true;
  }
} // initialize()

void
QueryGetPfcMultiAirportHistorical::findPfcMultiAirport(std::vector<tse::PfcMultiAirport*>& lstMA,
                                                       const LocCode& loc,
                                                       const DateTime& startDate,
                                                       const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, loc);
  substParm(2, startDate);
  substParm(3, endDate);
  substParm(4, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::PfcMultiAirport* ma = nullptr;
  tse::PfcMultiAirport* maPrev = nullptr;
  while ((row = res.nextRow()))
  {
    ma = QueryGetPfcMultiAirportHistoricalSQLStatement<
        QueryGetPfcMultiAirportHistorical>::mapRowToPfcMultiAirport(row, maPrev);
    if (ma != maPrev)
      lstMA.push_back(ma);

    maPrev = ma;
  }
  LOG4CXX_INFO(_logger,
               "GETPFCMULTIAIRPORTHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findPfcMultiAirport()
}
