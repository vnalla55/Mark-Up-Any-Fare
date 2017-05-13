//----------------------------------------------------------------------------
//  File:           QueryGetMaxStayRestriction.cpp
//  Description:    QueryGetMaxStayRestriction
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
#include "DBAccess/Queries/QueryGetMaxStayRestriction.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetMaxStayRestrictionSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetMaxStayRestriction::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetMaxStayRestriction"));
std::string QueryGetMaxStayRestriction::_baseSQL;
bool QueryGetMaxStayRestriction::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMaxStayRestriction> g_GetMaxStayRestriction;

const char*
QueryGetMaxStayRestriction::getQueryName() const
{
  return "GETMAXSTAYRESTRICTION";
}

void
QueryGetMaxStayRestriction::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMaxStayRestrictionSQLStatement<QueryGetMaxStayRestriction> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMAXSTAYRESTRICTION");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMaxStayRestriction::findMaxStayRestriction(std::vector<tse::MaxStayRestriction*>& lstMSR,
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
    lstMSR.push_back(QueryGetMaxStayRestrictionSQLStatement<
        QueryGetMaxStayRestriction>::mapRowToMaxStayRestriction(row));
  }
  LOG4CXX_INFO(_logger,
               "GETMAXSTAYRESTRICTION: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                   << " (" << stopCPU() << ")");
  res.freeResult();
} // findMaxStayRestriction()

///////////////////////////////////////////////////////////
//  QueryGetMaxStayRestrictionHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetMaxStayRestrictionHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetMaxStayRestrictionHistorical"));
std::string QueryGetMaxStayRestrictionHistorical::_baseSQL;
bool QueryGetMaxStayRestrictionHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMaxStayRestrictionHistorical> g_GetMaxStayRestrictionHistorical;

const char*
QueryGetMaxStayRestrictionHistorical::getQueryName() const
{
  return "GETMAXSTAYRESTRICTIONHISTORICAL";
}

void
QueryGetMaxStayRestrictionHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMaxStayRestrictionHistoricalSQLStatement<QueryGetMaxStayRestrictionHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMAXSTAYRESTRICTIONHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMaxStayRestrictionHistorical::findMaxStayRestriction(
    std::vector<tse::MaxStayRestriction*>& lstMSR,
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
    lstMSR.push_back(QueryGetMaxStayRestrictionHistoricalSQLStatement<
        QueryGetMaxStayRestrictionHistorical>::mapRowToMaxStayRestriction(row));
  }
  LOG4CXX_INFO(_logger,
               "GETMAXSTAYRESTRICTIONHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findMaxStayRestriction()
}
