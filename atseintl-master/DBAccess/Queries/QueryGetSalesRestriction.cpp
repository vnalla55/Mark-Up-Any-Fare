//----------------------------------------------------------------------------
//  File:           QueryGetSalesRestriction.cpp
//  Description:    QueryGetSalesRestriction
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
#include "DBAccess/Queries/QueryGetSalesRestriction.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetSalesRestrictionSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetSalesRestriction::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetSalesRestriction"));
std::string QueryGetSalesRestriction::_baseSQL;
bool QueryGetSalesRestriction::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSalesRestriction> g_GetSalesRestriction;

const char*
QueryGetSalesRestriction::getQueryName() const
{
  return "GETSALESRESTRICTION";
}

void
QueryGetSalesRestriction::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSalesRestrictionSQLStatement<QueryGetSalesRestriction> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSALESRESTRICTION");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetSalesRestriction::findSalesRestriction(std::vector<tse::SalesRestriction*>& lstSR,
                                               VendorCode& vendor,
                                               int itemNo)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char itemStr[15];
  sprintf(itemStr, "%d", itemNo);

  substParm(vendor, 1);
  substParm(itemStr, 2);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::SalesRestriction* sr = nullptr;
  tse::SalesRestriction* srPrev = nullptr;
  while ((row = res.nextRow()))
  {
    sr = QueryGetSalesRestrictionSQLStatement<QueryGetSalesRestriction>::mapRowToSalesRestriction(
        row, srPrev);
    if (sr != srPrev)
      lstSR.push_back(sr);

    srPrev = sr;
  }
  LOG4CXX_INFO(_logger,
               "GETSALESRESTRICTION: NumRows " << res.numRows() << " Time = " << stopTimer() << " ("
                                               << stopCPU() << ") mSecs");
  res.freeResult();
} // findSalesRestriction()

///////////////////////////////////////////////////////////
//  QueryGetSalesRestrictionHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetSalesRestrictionHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetSalesRestrictionHistorical"));
std::string QueryGetSalesRestrictionHistorical::_baseSQL;
bool QueryGetSalesRestrictionHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSalesRestrictionHistorical> g_GetSalesRestrictionHistorical;

const char*
QueryGetSalesRestrictionHistorical::getQueryName() const
{
  return "GETSALESRESTRICTIONHISTORICAL";
}

void
QueryGetSalesRestrictionHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSalesRestrictionHistoricalSQLStatement<QueryGetSalesRestrictionHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSALESRESTRICTIONHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetSalesRestrictionHistorical::findSalesRestriction(std::vector<tse::SalesRestriction*>& lstSR,
                                                         VendorCode& vendor,
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
  substParm(vendor, 5);
  substParm(itemStr, 6);
  substParm(7, startDate);
  substParm(8, endDate);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::SalesRestriction* sr = nullptr;
  tse::SalesRestriction* srPrev = nullptr;
  while ((row = res.nextRow()))
  {
    sr = QueryGetSalesRestrictionHistoricalSQLStatement<
        QueryGetSalesRestrictionHistorical>::mapRowToSalesRestriction(row, srPrev);
    if (sr != srPrev)
      lstSR.push_back(sr);

    srPrev = sr;
  }
  LOG4CXX_INFO(_logger,
               "GETSALESRESTRICTIONHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findSalesRestriction()
}
