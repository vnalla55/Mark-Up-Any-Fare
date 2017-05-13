//----------------------------------------------------------------------------
//  File:           QueryGetTransfersInfo1.cpp
//  Description:    QueryGetTransfersInfo1
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
#include "DBAccess/Queries/QueryGetTransfersInfo1.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetTransfersInfo1SQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetTransfersInfo1::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTransfersInfo1"));
std::string QueryGetTransfersInfo1::_baseSQL;
bool QueryGetTransfersInfo1::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTransfersInfo1> g_GetTransfersInfo1;

const char*
QueryGetTransfersInfo1::getQueryName() const
{
  return "GETTRANSFERSINFO1";
}

void
QueryGetTransfersInfo1::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTransfersInfo1SQLStatement<QueryGetTransfersInfo1> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTRANSFERSINFO1");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTransfersInfo1::findTransfersInfo1(std::vector<tse::TransfersInfo1*>& lstTI,
                                           VendorCode& vendor,
                                           int itemNo)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char strItemNo[20];
  sprintf(strItemNo, "%d", itemNo);

  substParm(vendor, 1);
  substParm(strItemNo, 2);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::TransfersInfo1* ti = nullptr;
  tse::TransfersInfo1* tiPrev = nullptr;
  while ((row = res.nextRow()))
  {
    ti = QueryGetTransfersInfo1SQLStatement<QueryGetTransfersInfo1>::mapRowToTransfersInfo1(row,
                                                                                            tiPrev);
    if (ti != tiPrev)
      lstTI.push_back(ti);

    tiPrev = ti;
  }
  LOG4CXX_INFO(_logger,
               "GETTRANSFERSINFO1: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                               << stopCPU() << ")");
  res.freeResult();
} // findTransfersInfo1()

///////////////////////////////////////////////////////////
//  QueryGetTransfersInfo1Historical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetTransfersInfo1Historical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetTransfersInfo1Historical"));
std::string QueryGetTransfersInfo1Historical::_baseSQL;
bool QueryGetTransfersInfo1Historical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTransfersInfo1Historical> g_GetTransfersInfo1Historical;

const char*
QueryGetTransfersInfo1Historical::getQueryName() const
{
  return "GETTRANSFERSINFO1HISTORICAL";
}

void
QueryGetTransfersInfo1Historical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTransfersInfo1HistoricalSQLStatement<QueryGetTransfersInfo1Historical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTRANSFERSINFO1HISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTransfersInfo1Historical::findTransfersInfo1(std::vector<tse::TransfersInfo1*>& lstTI,
                                                     VendorCode& vendor,
                                                     int itemNo,
                                                     const DateTime& startDate,
                                                     const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char strItemNo[20];
  sprintf(strItemNo, "%d", itemNo);

  substParm(vendor, 1);
  substParm(strItemNo, 2);
  substParm(3, startDate);
  substParm(4, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::TransfersInfo1* ti = nullptr;
  tse::TransfersInfo1* tiPrev = nullptr;
  while ((row = res.nextRow()))
  {
    ti = QueryGetTransfersInfo1HistoricalSQLStatement<
        QueryGetTransfersInfo1Historical>::mapRowToTransfersInfo1(row, tiPrev);
    if (ti != tiPrev)
      lstTI.push_back(ti);

    tiPrev = ti;
  }
  LOG4CXX_INFO(_logger,
               "GETTRANSFERSINFO1HISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findTransfersInfo1()
}
