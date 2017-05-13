//----------------------------------------------------------------------------
//  File:           QueryGetPfcCollectMeth.cpp
//  Description:    QueryGetPfcCollectMeth
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

#include "DBAccess/Queries/QueryGetPfcCollectMeth.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetPfcCollectMethSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetPfcCollectMeth::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetPfcCollectMeth"));
std::string QueryGetPfcCollectMeth::_baseSQL;
bool QueryGetPfcCollectMeth::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetPfcCollectMeth> g_GetPfcCollectMeth;

const char*
QueryGetPfcCollectMeth::getQueryName() const
{
  return "GETPFCCOLLECTMETH";
}

void
QueryGetPfcCollectMeth::initialize()
{
  if (!_isInitialized)
  {
    QueryGetPfcCollectMethSQLStatement<QueryGetPfcCollectMeth> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETPFCCOLLECTMETH");
    substTableDef(&_baseSQL);

    _isInitialized = true;
  }
} // initialize()

void
QueryGetPfcCollectMeth::findPfcCollectMeth(std::vector<tse::PfcCollectMeth*>& lstCM,
                                           const CarrierCode& carrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, carrier);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::PfcCollectMeth* cm = nullptr;
  tse::PfcCollectMeth* cmPrev = nullptr;
  while ((row = res.nextRow()))
  {
    cm = QueryGetPfcCollectMethSQLStatement<QueryGetPfcCollectMeth>::mapRowToPfcCollectMeth(row,
                                                                                            cmPrev);
    if (cm != cmPrev)
      lstCM.push_back(cm);

    cmPrev = cm;
  }
  LOG4CXX_INFO(_logger,
               "GETPFCCOLLECTMETH: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                               << stopCPU() << ")");
  res.freeResult();
} // findPfcCollectMeth()

///////////////////////////////////////////////////////////
//
//  QueryGetAllPfcCollectMeth
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllPfcCollectMeth::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllPfcCollectMeth"));
std::string QueryGetAllPfcCollectMeth::_baseSQL;
bool QueryGetAllPfcCollectMeth::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllPfcCollectMeth> g_GetAllPfcCollectMeth;

const char*
QueryGetAllPfcCollectMeth::getQueryName() const
{
  return "GETALLPFCCOLLECTMETH";
}

void
QueryGetAllPfcCollectMeth::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllPfcCollectMethSQLStatement<QueryGetAllPfcCollectMeth> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLPFCCOLLECTMETH");
    substTableDef(&_baseSQL);

    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllPfcCollectMeth::findAllPfcCollectMeth(std::vector<tse::PfcCollectMeth*>& lstCM)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::PfcCollectMeth* cm = nullptr;
  tse::PfcCollectMeth* cmPrev = nullptr;
  while ((row = res.nextRow()))
  {
    cm = QueryGetAllPfcCollectMethSQLStatement<QueryGetAllPfcCollectMeth>::mapRowToPfcCollectMeth(
        row, cmPrev);
    if (cm != cmPrev)
      lstCM.push_back(cm);

    cmPrev = cm;
  }
  LOG4CXX_INFO(_logger,
               "GETALLPFCCOLLECTMETH: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                  << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllPfcCollectMeth()

///////////////////////////////////////////////////////////
//
//  QueryGetPfcCollectMethHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetPfcCollectMethHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetPfcCollectMethHistorical"));
std::string QueryGetPfcCollectMethHistorical::_baseSQL;
bool QueryGetPfcCollectMethHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetPfcCollectMethHistorical> g_GetPfcCollectMethHistorical;

const char*
QueryGetPfcCollectMethHistorical::getQueryName() const
{
  return "GETPFCCOLLECTMETHHISTORICAL";
}

void
QueryGetPfcCollectMethHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetPfcCollectMethHistoricalSQLStatement<QueryGetPfcCollectMethHistorical> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETPFCCOLLECTMETHHISTORICAL");
    substTableDef(&_baseSQL);

    _isInitialized = true;
  }
} // initialize()

void
QueryGetPfcCollectMethHistorical::findPfcCollectMeth(std::vector<tse::PfcCollectMeth*>& lstCM,
                                                     const CarrierCode& carrier,
                                                     const DateTime& startDate,
                                                     const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, carrier);
  substParm(2, startDate);
  substParm(3, endDate);
  substParm(4, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::PfcCollectMeth* cm = nullptr;
  tse::PfcCollectMeth* cmPrev = nullptr;
  while ((row = res.nextRow()))
  {
    cm = QueryGetPfcCollectMethHistoricalSQLStatement<
        QueryGetPfcCollectMethHistorical>::mapRowToPfcCollectMeth(row, cmPrev);
    if (cm != cmPrev)
      lstCM.push_back(cm);

    cmPrev = cm;
  }
  LOG4CXX_INFO(_logger,
               "GETPFCCOLLECTMETHHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findPfcCollectMeth()
}
