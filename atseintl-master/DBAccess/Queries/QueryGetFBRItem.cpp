//----------------------------------------------------------------------------
//  File:           QueryGetFBRItem.cpp
//  Description:    QueryGetFBRItem
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
#include "DBAccess/Queries/QueryGetFBRItem.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFBRItemSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetFBRItem::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetFBRItem"));
std::string QueryGetFBRItem::_baseSQL;
bool QueryGetFBRItem::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFBRItem> g_GetFBRItem;

const char*
QueryGetFBRItem::getQueryName() const
{
  return "GETFBRITEM";
}

void
QueryGetFBRItem::initialize()
{
  if (UNLIKELY(!_isInitialized))
  {
    QueryGetFBRItemSQLStatement<QueryGetFBRItem> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFBRITEM");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetFBRItem::findFareByRuleItemInfo(std::vector<FareByRuleItemInfo*>& fbrs,
                                        const VendorCode& vendor,
                                        int itemNumber)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char itemStr[15];
  sprintf(itemStr, "%d", itemNumber);

  substParm(vendor, 1);
  substParm(itemStr, 2);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    fbrs.push_back(QueryGetFBRItemSQLStatement<QueryGetFBRItem>::mapRowToFareByRuleItemInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETFBRITEM: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                        << stopCPU() << ")");
  res.freeResult();
} // findFareByRuleItemInfo()

///////////////////////////////////////////////////////////
//  QueryGetFBRItemHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetFBRItemHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetFBRItemHistorical"));
std::string QueryGetFBRItemHistorical::_baseSQL;
bool QueryGetFBRItemHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFBRItemHistorical> g_GetFBRItemHistorical;

const char*
QueryGetFBRItemHistorical::getQueryName() const
{
  return "GETFBRITEMHISTORICAL";
}

void
QueryGetFBRItemHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFBRItemHistoricalSQLStatement<QueryGetFBRItemHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFBRITEMHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetFBRItemHistorical::findFareByRuleItemInfo(std::vector<FareByRuleItemInfo*>& fbrs,
                                                  const VendorCode& vendor,
                                                  int itemNumber,
                                                  const DateTime& startDate,
                                                  const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char itemStr[15];
  sprintf(itemStr, "%d", itemNumber);

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
    fbrs.push_back(QueryGetFBRItemHistoricalSQLStatement<
        QueryGetFBRItemHistorical>::mapRowToFareByRuleItemInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETFBRITEMHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                  << " (" << stopCPU() << ")");
  res.freeResult();
} // findFareByRuleItemInfo()
}
