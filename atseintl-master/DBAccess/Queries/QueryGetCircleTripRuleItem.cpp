//----------------------------------------------------------------------------
//  File:           QueryGetCircleTripRuleItem.cpp
//  Description:    QueryGetCircleTripRuleItem
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
#include "DBAccess/Queries/QueryGetCircleTripRuleItem.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCircleTripRuleItemSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetCircleTripRuleItem::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetCircleTripRuleItem"));
std::string QueryGetCircleTripRuleItem::_baseSQL;
bool QueryGetCircleTripRuleItem::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCircleTripRuleItem> g_GetCircleTripRuleItem;

const char*
QueryGetCircleTripRuleItem::getQueryName() const
{
  return "GETCIRCLETRIPRULEITEM";
}

void
QueryGetCircleTripRuleItem::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCircleTripRuleItemSQLStatement<QueryGetCircleTripRuleItem> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCIRCLETRIPRULEITEM");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetCircleTripRuleItem::findCircleTripRuleItem(std::vector<tse::CircleTripRuleItem*>& circles,
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
    circles.push_back(QueryGetCircleTripRuleItemSQLStatement<
        QueryGetCircleTripRuleItem>::mapRowToCircleTripRuleItem(row));
  }
  LOG4CXX_INFO(_logger,
               "GETCIRCLETRIPRULEITEM: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                   << " (" << stopCPU() << ")");
  res.freeResult();
} // findCircleTripRuleItem()

///////////////////////////////////////////////////////////
//  QueryGetCircleTripRuleItemHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetCircleTripRuleItemHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetCircleTripRuleItemHistorical"));
std::string QueryGetCircleTripRuleItemHistorical::_baseSQL;
bool QueryGetCircleTripRuleItemHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCircleTripRuleItemHistorical> g_GetCircleTripRuleItemHistorical;

const char*
QueryGetCircleTripRuleItemHistorical::getQueryName() const
{
  return "GETCIRCLETRIPRULEITEMHISTORICAL";
}

void
QueryGetCircleTripRuleItemHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCircleTripRuleItemHistoricalSQLStatement<QueryGetCircleTripRuleItemHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCIRCLETRIPRULEITEMHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetCircleTripRuleItemHistorical::findCircleTripRuleItem(
    std::vector<tse::CircleTripRuleItem*>& circles,
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
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    circles.push_back(QueryGetCircleTripRuleItemHistoricalSQLStatement<
        QueryGetCircleTripRuleItemHistorical>::mapRowToCircleTripRuleItem(row));
  }
  LOG4CXX_INFO(_logger,
               "GETCIRCLETRIPRULEITEMHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findCircleTripRuleItem()
}
