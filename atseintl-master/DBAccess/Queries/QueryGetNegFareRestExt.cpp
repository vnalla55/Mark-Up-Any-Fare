//----------------------------------------------------------------------------
//  File:           QueryGetNegFareRestExt.cpp
//  Description:    QueryGetNegFareRestExt
//  Created:        9/9/2010
// Authors:         Artur Krezel
//
//  Updates:
//
// � 2010, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetNegFareRestExt.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetNegFareRestExtSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetNegFareRestExt::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetNegFareRestExt"));
std::string QueryGetNegFareRestExt::_baseSQL;
bool QueryGetNegFareRestExt::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetNegFareRestExt> g_GetNegFareRestExt;

const char*
QueryGetNegFareRestExt::getQueryName() const
{
  return "GETNEGFARERESTEXT";
}

void
QueryGetNegFareRestExt::initialize()
{
  if (!_isInitialized)
  {
    QueryGetNegFareRestExtSQLStatement<QueryGetNegFareRestExt> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETNEGFARERESTEXT");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetNegFareRestExt::findNegFareRestExt(std::vector<tse::NegFareRestExt*>& lstNFR,
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
  while ((row = res.nextRow()))
  {
    lstNFR.push_back(
        QueryGetNegFareRestExtSQLStatement<QueryGetNegFareRestExt>::mapRowToNegFareRestExt(row));
  }
  LOG4CXX_INFO(_logger,
               "GETNEGFARERESTEXT: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                               << stopCPU() << ")");
  res.freeResult();
} // findNegFareRestExt()

///////////////////////////////////////////////////////////
//  QueryGetNegFareRestExtHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetNegFareRestExtHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetNegFareRestExtHistorical"));
std::string QueryGetNegFareRestExtHistorical::_baseSQL;
bool QueryGetNegFareRestExtHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetNegFareRestExtHistorical> g_GetNegFareRestExtHistorical;

const char*
QueryGetNegFareRestExtHistorical::getQueryName() const
{
  return "GETNEGFARERESTEXTHISTORICAL";
}

void
QueryGetNegFareRestExtHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetNegFareRestExtHistoricalSQLStatement<QueryGetNegFareRestExtHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETNEGFARERESTEXTHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetNegFareRestExtHistorical::findNegFareRestExt(std::vector<tse::NegFareRestExt*>& lstNFR,
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
  while ((row = res.nextRow()))
  {
    lstNFR.push_back(QueryGetNegFareRestExtHistoricalSQLStatement<
        QueryGetNegFareRestExtHistorical>::mapRowToNegFareRestExt(row));
  }
  LOG4CXX_INFO(_logger,
               "GETNEGFARERESTEXTHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findNegFareRestExt()
}
