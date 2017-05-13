//----------------------------------------------------------------------------
//  File:           QueryGetOpenJawRestriction.cpp
//  Description:    QueryGetOpenJawRestriction
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
#include "DBAccess/Queries/QueryGetOpenJawRestriction.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetOpenJawRestrictionSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetOpenJawRestriction::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetOpenJawRestriction"));
std::string QueryGetOpenJawRestriction::_baseSQL;
bool QueryGetOpenJawRestriction::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetOpenJawRestriction> g_GetOpenJawRestriction;

const char*
QueryGetOpenJawRestriction::getQueryName() const
{
  return "GETOPENJAWRESTRICTION";
}

void
QueryGetOpenJawRestriction::initialize()
{
  if (!_isInitialized)
  {
    QueryGetOpenJawRestrictionSQLStatement<QueryGetOpenJawRestriction> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETOPENJAWRESTRICTION");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetOpenJawRestriction::findOpenJawRestriction(std::vector<OpenJawRestriction*>& openJawRes,
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
    openJawRes.push_back(QueryGetOpenJawRestrictionSQLStatement<
        QueryGetOpenJawRestriction>::mapRowToOpenJawRestriction(row));
  }
  LOG4CXX_INFO(_logger,
               "GETOPENJAWRESTRICTION: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                   << " (" << stopCPU() << ")");
  res.freeResult();
} // findOpenJawRestriction()

///////////////////////////////////////////////////////////
//  QueryGetOpenJawRestrictionHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetOpenJawRestrictionHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetOpenJawRestrictionHistorical"));
std::string QueryGetOpenJawRestrictionHistorical::_baseSQL;
bool QueryGetOpenJawRestrictionHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetOpenJawRestrictionHistorical> g_GetOpenJawRestrictionHistorical;

const char*
QueryGetOpenJawRestrictionHistorical::getQueryName() const
{
  return "GETOPENJAWRESTRICTIONHISTORICAL";
}

void
QueryGetOpenJawRestrictionHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetOpenJawRestrictionHistoricalSQLStatement<QueryGetOpenJawRestrictionHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETOPENJAWRESTRICTIONHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetOpenJawRestrictionHistorical::findOpenJawRestriction(
    std::vector<OpenJawRestriction*>& openJawRes,
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
    openJawRes.push_back(QueryGetOpenJawRestrictionHistoricalSQLStatement<
        QueryGetOpenJawRestrictionHistorical>::mapRowToOpenJawRestriction(row));
  }
  LOG4CXX_INFO(_logger,
               "GETOPENJAWRESTRICTIONHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findOpenJawRestriction()
}
