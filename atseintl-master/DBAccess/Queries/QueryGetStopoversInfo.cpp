//----------------------------------------------------------------------------
//  File:           QueryGetStopoversInfo.cpp
//  Description:    QueryGetStopoversInfo
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
#include "DBAccess/Queries/QueryGetStopoversInfo.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetStopoversInfoSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetStopoversInfo::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetStopoversInfo"));
std::string QueryGetStopoversInfo::_baseSQL;
bool QueryGetStopoversInfo::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetStopoversInfo> g_GetStopoversInfo;

const char*
QueryGetStopoversInfo::getQueryName() const
{
  return "GETSTOPOVERSINFO";
}

void
QueryGetStopoversInfo::initialize()
{
  if (!_isInitialized)
  {
    QueryGetStopoversInfoSQLStatement<QueryGetStopoversInfo> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSTOPOVERSINFO");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

namespace
{
bool
SortSIByChgID(const StopoversInfoSeg* sis1, const StopoversInfoSeg* sis2)
{
  return (sis1->chargeInd() < sis2->chargeInd());
}
}

void
QueryGetStopoversInfo::findStopoversInfo(std::vector<tse::StopoversInfo*>& lstSI,
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

  tse::StopoversInfo* si = nullptr;
  tse::StopoversInfo* siPrev = nullptr;
  while ((row = res.nextRow()))
  {
    si = QueryGetStopoversInfoSQLStatement<QueryGetStopoversInfo>::mapRowToStopoversInfo(row,
                                                                                         siPrev);
    if (si != siPrev)
    {
      if (siPrev)
        std::sort(siPrev->segs().begin(), siPrev->segs().end(), SortSIByChgID);
      lstSI.push_back(si);
    }

    siPrev = si;
  }
  if (si)
    std::sort(si->segs().begin(), si->segs().end(), SortSIByChgID);

  LOG4CXX_INFO(_logger,
               "GETSTOPOVERSINFO: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                              << stopCPU() << ")");
  res.freeResult();
} // findStopoversInfo()

///////////////////////////////////////////////////////////
//  QueryGetStopoversInfoHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetStopoversInfoHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetStopoversInfoHistorical"));
std::string QueryGetStopoversInfoHistorical::_baseSQL;
bool QueryGetStopoversInfoHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetStopoversInfoHistorical> g_GetStopoversInfoHistorical;

const char*
QueryGetStopoversInfoHistorical::getQueryName() const
{
  return "GETSTOPOVERSINFOHISTORICAL";
}

void
QueryGetStopoversInfoHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetStopoversInfoHistoricalSQLStatement<QueryGetStopoversInfoHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSTOPOVERSINFOHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetStopoversInfoHistorical::findStopoversInfo(std::vector<tse::StopoversInfo*>& lstSI,
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

  tse::StopoversInfo* si = nullptr;
  tse::StopoversInfo* siPrev = nullptr;
  while ((row = res.nextRow()))
  {
    si = QueryGetStopoversInfoHistoricalSQLStatement<
        QueryGetStopoversInfoHistorical>::mapRowToStopoversInfo(row, siPrev);
    if (si != siPrev)
      lstSI.push_back(si);

    siPrev = si;
  }
  LOG4CXX_INFO(_logger,
               "GETSTOPOVERSINFOHISTORICAL: NumRows = " << res.numRows() << " Time = "
                                                        << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findStopoversInfo()
}
