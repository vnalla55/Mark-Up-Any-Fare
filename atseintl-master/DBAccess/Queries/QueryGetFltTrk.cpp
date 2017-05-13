//----------------------------------------------------------------------------
//  File:           QueryGetFltTrk.cpp
//  Description:    QueryGetFltTrk
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
#include "DBAccess/Queries/QueryGetFltTrk.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFltTrkSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetFltTrk::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetFltTrk"));
std::string QueryGetFltTrk::_baseSQL;
bool QueryGetFltTrk::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFltTrk> g_GetFltTrk;

const char*
QueryGetFltTrk::getQueryName() const
{
  return "GETFLTTRK";
}

void
QueryGetFltTrk::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFltTrkSQLStatement<QueryGetFltTrk> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFLTTRK");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetFltTrk::findFltTrkCntryGrp(std::vector<tse::FltTrkCntryGrp*>& trks, const CarrierCode& cxr)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, cxr);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FltTrkCntryGrp* trkPrev = nullptr;
  FltTrkCntryGrp* trk = nullptr;
  while ((row = res.nextRow()))
  {
    trk = QueryGetFltTrkSQLStatement<QueryGetFltTrk>::mapRowToFltTrkCntryGrp(row, trkPrev);
    if (trkPrev != trk)
    {
      trks.push_back(trk);
    }
    trkPrev = trk;
  }
  LOG4CXX_INFO(_logger,
               "GETFLTTRK: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                       << stopCPU() << ")");
  res.freeResult();
} // findFltTrkCntryGrp()

///////////////////////////////////////////////////////////
//
//  QueryGetAllFltTrk
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllFltTrk::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllFltTrk"));
std::string QueryGetAllFltTrk::_baseSQL;
bool QueryGetAllFltTrk::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllFltTrk> g_GetAllFltTrk;

const char*
QueryGetAllFltTrk::getQueryName() const
{
  return "GETALLFLTTRK";
}

void
QueryGetAllFltTrk::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllFltTrkSQLStatement<QueryGetAllFltTrk> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLFLTTRK");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllFltTrk::findAllFltTrkCntryGrp(std::vector<tse::FltTrkCntryGrp*>& trks)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FltTrkCntryGrp* trkPrev = nullptr;
  FltTrkCntryGrp* trk = nullptr;
  while ((row = res.nextRow()))
  {
    trk = QueryGetAllFltTrkSQLStatement<QueryGetAllFltTrk>::mapRowToFltTrkCntryGrp(row, trkPrev);
    if (trkPrev != trk)
    {
      trks.push_back(trk);
    }
    trkPrev = trk;
  }
  LOG4CXX_INFO(_logger,
               "GETALLFLTTRK: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                          << stopCPU() << ")");
  res.freeResult();
} // findAllFltTrkCntryGrp()

///////////////////////////////////////////////////////////
//
//  QueryGetFltTrkHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetFltTrkHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetFltTrkHistorical"));
std::string QueryGetFltTrkHistorical::_baseSQL;
bool QueryGetFltTrkHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFltTrkHistorical> g_GetFltTrkHistorical;

const char*
QueryGetFltTrkHistorical::getQueryName() const
{
  return "GETFLTTRKHISTORICAL";
}

void
QueryGetFltTrkHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFltTrkHistoricalSQLStatement<QueryGetFltTrkHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFLTTRKHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetFltTrkHistorical::findFltTrkCntryGrp(std::vector<tse::FltTrkCntryGrp*>& trks,
                                             const CarrierCode& cxr)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, cxr);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FltTrkCntryGrp* trkPrev = nullptr;
  FltTrkCntryGrp* trk = nullptr;
  while ((row = res.nextRow()))
  {
    trk = QueryGetFltTrkHistoricalSQLStatement<QueryGetFltTrkHistorical>::mapRowToFltTrkCntryGrp(
        row, trkPrev);
    if (trkPrev != trk)
    {
      trks.push_back(trk);
    }
    trkPrev = trk;
  }
  LOG4CXX_INFO(_logger,
               "GETFLTTRKHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                 << " (" << stopCPU() << ")");
  res.freeResult();
} // findFltTrkCntryGrp()

///////////////////////////////////////////////////////////
//
//  QueryGetAllFltTrkHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllFltTrkHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllFltTrkHistorical"));
std::string QueryGetAllFltTrkHistorical::_baseSQL;
bool QueryGetAllFltTrkHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllFltTrkHistorical> g_GetAllFltTrkHistorical;

const char*
QueryGetAllFltTrkHistorical::getQueryName() const
{
  return "GETALLFLTTRKHISTORICAL";
}

void
QueryGetAllFltTrkHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllFltTrkHistoricalSQLStatement<QueryGetAllFltTrkHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLFLTTRKHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllFltTrkHistorical::findAllFltTrkCntryGrp(std::vector<tse::FltTrkCntryGrp*>& trks)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FltTrkCntryGrp* trkPrev = nullptr;
  FltTrkCntryGrp* trk = nullptr;
  while ((row = res.nextRow()))
  {
    trk = QueryGetAllFltTrkHistoricalSQLStatement<
        QueryGetAllFltTrkHistorical>::mapRowToFltTrkCntryGrp(row, trkPrev);
    if (trkPrev != trk)
    {
      trks.push_back(trk);
    }
    trkPrev = trk;
  }
  LOG4CXX_INFO(_logger,
               "GETALLFLTTRKHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                    << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllFltTrkCntryGrp()
}
