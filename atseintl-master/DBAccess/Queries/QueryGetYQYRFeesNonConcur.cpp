//----------------------------------------------------------------------------
//  File:           QueryGetYQYRFeesNonConcur.cpp
//  Description:    QueryGetYQYRFeesNonConcur
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

#include "DBAccess/Queries/QueryGetYQYRFeesNonConcur.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetYQYRFeesNonConcurSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetYQYRFeesNonConcur::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetYQYRFeesNonConcur"));
std::string QueryGetYQYRFeesNonConcur::_baseSQL;
bool QueryGetYQYRFeesNonConcur::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetYQYRFeesNonConcur> g_GetYQYRFeesNonConcur;

const char*
QueryGetYQYRFeesNonConcur::getQueryName() const
{
  return "GETYQYRFEESNONCONCUR";
}

void
QueryGetYQYRFeesNonConcur::initialize()
{
  if (!_isInitialized)
  {
    QueryGetYQYRFeesNonConcurSQLStatement<QueryGetYQYRFeesNonConcur> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETYQYRFEESNONCONCUR");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetYQYRFeesNonConcur::findYQYRFeesNonConcur(std::vector<tse::YQYRFeesNonConcur*>& yqyrFNCs,
                                                 CarrierCode& carrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  substParm(carrier, 1);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::YQYRFeesNonConcur* fnc = nullptr;
  tse::YQYRFeesNonConcur* fncPrev = nullptr;
  while ((row = res.nextRow()))
  {
    fnc =
        QueryGetYQYRFeesNonConcurSQLStatement<QueryGetYQYRFeesNonConcur>::mapRowToYQYRFeesNonConcur(
            row, fncPrev);
    if (fnc != fncPrev)
      yqyrFNCs.push_back(fnc);

    fncPrev = fnc;
  }
  LOG4CXX_INFO(_logger,
               "GETYQYRFEESNONCONCUR: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                  << " (" << stopCPU() << ")");
  res.freeResult();
} // findYQYRFeesNonConcur()

///////////////////////////////////////////////////////////
//  QueryGetAllYQYRFeesNonConcur
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllYQYRFeesNonConcur::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllYQYRFeesNonConcur"));
std::string QueryGetAllYQYRFeesNonConcur::_baseSQL;
bool QueryGetAllYQYRFeesNonConcur::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllYQYRFeesNonConcur> g_GetAllYQYRFeesNonConcur;

const char*
QueryGetAllYQYRFeesNonConcur::getQueryName() const
{
  return "GETALLYQYRFEESNONCONCUR";
}

void
QueryGetAllYQYRFeesNonConcur::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllYQYRFeesNonConcurSQLStatement<QueryGetAllYQYRFeesNonConcur> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLYQYRFEESNONCONCUR");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllYQYRFeesNonConcur::findAllYQYRFeesNonConcur(
    std::vector<tse::YQYRFeesNonConcur*>& yqyrFNCs)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::YQYRFeesNonConcur* fnc = nullptr;
  tse::YQYRFeesNonConcur* fncPrev = nullptr;
  while ((row = res.nextRow()))
  {
    fnc = QueryGetAllYQYRFeesNonConcurSQLStatement<
        QueryGetAllYQYRFeesNonConcur>::mapRowToYQYRFeesNonConcur(row, fncPrev);
    if (fnc != fncPrev)
      yqyrFNCs.push_back(fnc);

    fncPrev = fnc;
  }
  LOG4CXX_INFO(_logger,
               "GETALLYQYRFEESNONCONCUR: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                     << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllYQYRFeesNonConcur()

///////////////////////////////////////////////////////////
//  QueryGetYQYRFeesNonConcurHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetYQYRFeesNonConcurHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetYQYRFeesNonConcurHistorical"));
std::string QueryGetYQYRFeesNonConcurHistorical::_baseSQL;
bool QueryGetYQYRFeesNonConcurHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetYQYRFeesNonConcurHistorical>
g_QueryGetYQYRFeesNonConcurHistorical;

const char*
QueryGetYQYRFeesNonConcurHistorical::getQueryName() const
{
  return "GETYQYRFEESNONCONCURHISTORICAL";
}

void
QueryGetYQYRFeesNonConcurHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetYQYRFeesNonConcurHistoricalSQLStatement<QueryGetYQYRFeesNonConcurHistorical>
    sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETYQYRFEESNONCONCURHISTORICAL");
    substTableDef(&_baseSQL);

    _isInitialized = true;
  }
} // initialize()

void
QueryGetYQYRFeesNonConcurHistorical::findYQYRFeesNonConcur(
    std::vector<tse::YQYRFeesNonConcur*>& yqyrFNCs,
    CarrierCode& carrier,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, carrier);
  substParm(2, startDate);
  substParm(3, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::YQYRFeesNonConcur* fnc = nullptr;
  tse::YQYRFeesNonConcur* fncPrev = nullptr;
  while ((row = res.nextRow()))
  {
    fnc = QueryGetYQYRFeesNonConcurHistoricalSQLStatement<
        QueryGetYQYRFeesNonConcurHistorical>::mapRowToYQYRFeesNonConcur(row, fncPrev);
    if (fnc != fncPrev)
      yqyrFNCs.push_back(fnc);

    fncPrev = fnc;
  }
  LOG4CXX_INFO(_logger,
               "GETYQYRFEESNONCONCURHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findYQYRFeesNonConcur()
}
