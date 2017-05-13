//----------------------------------------------------------------------------
//  File:           QueryGetMinStayRestriction.cpp
//  Description:    QueryGetMinStayRestriction
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
#include "DBAccess/Queries/QueryGetMinStayRestriction.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetMinStayRestrictionSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetMinStayRestriction::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetMinStayRestriction"));
std::string QueryGetMinStayRestriction::_baseSQL;
bool QueryGetMinStayRestriction::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMinStayRestriction> g_GetMinStayRestriction;

const char*
QueryGetMinStayRestriction::getQueryName() const
{
  return "GETMINSTAYRESTRICTION";
}

void
QueryGetMinStayRestriction::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMinStayRestrictionSQLStatement<QueryGetMinStayRestriction> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMINSTAYRESTRICTION");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMinStayRestriction::findMinStayRestriction(std::vector<tse::MinStayRestriction*>& lstMSR,
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
    lstMSR.push_back(QueryGetMinStayRestrictionSQLStatement<
        QueryGetMinStayRestriction>::mapRowToMinStayRestriction(row));
  }
  LOG4CXX_INFO(_logger,
               "GETMINSTAYRESTRICTION: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                   << " (" << stopCPU() << ")");
  res.freeResult();
} // findMinStayRestriction()

///////////////////////////////////////////////////////////
//  QueryGetMinStayRestrictionHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetMinStayRestrictionHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetMinStayRestrictionHistorical"));
std::string QueryGetMinStayRestrictionHistorical::_baseSQL;
bool QueryGetMinStayRestrictionHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMinStayRestrictionHistorical> g_GetMinStayRestrictionHistorical;

const char*
QueryGetMinStayRestrictionHistorical::getQueryName() const
{
  return "GETMINSTAYRESTRICTIONHISTORICAL";
}

void
QueryGetMinStayRestrictionHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMinStayRestrictionHistoricalSQLStatement<QueryGetMinStayRestrictionHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMINSTAYRESTRICTIONHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMinStayRestrictionHistorical::findMinStayRestriction(
    std::vector<tse::MinStayRestriction*>& lstMSR,
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
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstMSR.push_back(QueryGetMinStayRestrictionSQLStatement<
        QueryGetMinStayRestrictionHistorical>::mapRowToMinStayRestriction(row));
  }
  LOG4CXX_INFO(_logger,
               "GETMINSTAYRESTRICTIONHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findMinStayRestriction()
}
