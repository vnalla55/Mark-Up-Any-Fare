//----------------------------------------------------------------------------
//  File:           QueryGetAccompaniedTravelInfo.cpp
//  Description:    QueryGetAccompaniedTravelInfo
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
#include "DBAccess/Queries/QueryGetAccompaniedTravelInfo.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetAccompaniedTravelInfoSQLStatement.h"

namespace tse
{
///////////////////////////////////////////////////////////
//  QueryGetAccompaniedTravelInfo
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAccompaniedTravelInfo::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAccompaniedTravelInfo"));
std::string QueryGetAccompaniedTravelInfo::_baseSQL;
bool QueryGetAccompaniedTravelInfo::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAccompaniedTravelInfo> g_GetAccompaniedTravelInfo;

const char*
QueryGetAccompaniedTravelInfo::getQueryName() const
{
  return "GETACCOMPANIEDTRAVELINFO";
}

void
QueryGetAccompaniedTravelInfo::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAccompaniedTravelInfoSQLStatement<QueryGetAccompaniedTravelInfo> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETACCOMPANIEDTRAVELINFO");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAccompaniedTravelInfo::findAccompaniedTravelInfo(
    std::vector<tse::AccompaniedTravelInfo*>& lstATI, VendorCode& vendor, int itemNo)
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

  tse::AccompaniedTravelInfo* ati = nullptr;
  tse::AccompaniedTravelInfo* atiPrev = nullptr;
  while ((row = res.nextRow()))
  {
    ati = QueryGetAccompaniedTravelInfoSQLStatement<
        QueryGetAccompaniedTravelInfo>::mapRowToAccompaniedTravelInfo(row, atiPrev);
    if (ati != atiPrev)
      lstATI.push_back(ati);

    atiPrev = ati;
  }
  LOG4CXX_INFO(_logger,
               "GETACCOMPANIEDTRAVELINFO: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ")");
  res.freeResult();
} // findAccompaniedTravelInfo()

///////////////////////////////////////////////////////////
//  QueryGetAccompaniedTravelInfoHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAccompaniedTravelInfoHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.QueryGetAccompaniedTravelInfoHistorical"));
std::string QueryGetAccompaniedTravelInfoHistorical::_baseSQL;
bool QueryGetAccompaniedTravelInfoHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAccompaniedTravelInfoHistorical>
g_GetAccompaniedTravelInfoHistorical;

const char*
QueryGetAccompaniedTravelInfoHistorical::getQueryName() const
{
  return "GETACCOMPANIEDTRAVELINFOHISTORICAL";
}

void
QueryGetAccompaniedTravelInfoHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAccompaniedTravelInfoHistoricalSQLStatement<QueryGetAccompaniedTravelInfoHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETACCOMPANIEDTRAVELINFOHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAccompaniedTravelInfoHistorical::findAccompaniedTravelInfo(
    std::vector<tse::AccompaniedTravelInfo*>& lstATI,
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

  tse::AccompaniedTravelInfo* ati = nullptr;
  tse::AccompaniedTravelInfo* atiPrev = nullptr;
  while ((row = res.nextRow()))
  {
    ati = QueryGetAccompaniedTravelInfoHistoricalSQLStatement<
        QueryGetAccompaniedTravelInfoHistorical>::mapRowToAccompaniedTravelInfo(row, atiPrev);
    if (ati != atiPrev)
      lstATI.push_back(ati);

    atiPrev = ati;
  }
  LOG4CXX_INFO(_logger,
               "GETACCOMPANIEDTRAVELINFOHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findAccompaniedTravelInfo()
}
