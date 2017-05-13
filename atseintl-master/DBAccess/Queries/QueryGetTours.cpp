//----------------------------------------------------------------------------
//  File:           QueryGetTours.cpp
//  Description:    QueryGetTours
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
#include "DBAccess/Queries/QueryGetTours.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetToursSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetTours::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTours"));
std::string QueryGetTours::_baseSQL;
bool QueryGetTours::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTours> g_GetTours;

const char*
QueryGetTours::getQueryName() const
{
  return "GETTOURS";
}

void
QueryGetTours::initialize()
{
  if (!_isInitialized)
  {
    QueryGetToursSQLStatement<QueryGetTours> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTOURS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTours::findTours(std::vector<tse::Tours*>& tours, const VendorCode& vendor, int itemNo)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, vendor);
  substParm(2, itemNo);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  Tours* prev = nullptr;
  while ((row = res.nextRow()))
  {
    Tours* rec = QueryGetToursSQLStatement<QueryGetTours>::mapRowToTours(row, prev);
    if (prev != rec)
    {
      tours.push_back(rec);
      prev = rec;
    }
  }
  LOG4CXX_INFO(_logger,
               "GETTOURS: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                      << stopCPU() << ")");
  res.freeResult();
} // findTours()

///////////////////////////////////////////////////////////
//  QueryGetToursHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetToursHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetToursHistorical"));
std::string QueryGetToursHistorical::_baseSQL;
bool QueryGetToursHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetToursHistorical> g_GetToursHistorical;

const char*
QueryGetToursHistorical::getQueryName() const
{
  return "GETTOURSHISTORICAL";
}

void
QueryGetToursHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetToursHistoricalSQLStatement<QueryGetToursHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTOURSHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetToursHistorical::findTours(std::vector<tse::Tours*>& tours,
                                   const VendorCode& vendor,
                                   int itemNo,
                                   const DateTime& startDate,
                                   const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char itemStr[15];
  sprintf(itemStr, "%d", itemNo);

  substParm(vendor, 1);
  substParm(itemStr, 2);
  substParm(3, startDate);
  substParm(4, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  Tours* prev = nullptr;
  while ((row = res.nextRow()))
  {
    Tours* rec =
        QueryGetToursHistoricalSQLStatement<QueryGetToursHistorical>::mapRowToTours(row, prev);
    if (prev != rec)
    {
      tours.push_back(rec);
      prev = rec;
    }
  }
  LOG4CXX_INFO(_logger,
               "GETTOURSHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                << " (" << stopCPU() << ")");
  res.freeResult();
} // findTours()
}
