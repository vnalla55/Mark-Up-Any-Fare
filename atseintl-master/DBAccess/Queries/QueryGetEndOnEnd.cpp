//----------------------------------------------------------------------------
//  File:           QueryGetEndOnEnd.cpp
//  Description:    QueryGetEndOnEnd
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
#include "DBAccess/Queries/QueryGetEndOnEnd.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetEndOnEndSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetEndOnEnd::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetEndOnEnd"));
std::string QueryGetEndOnEnd::_baseSQL;
bool QueryGetEndOnEnd::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetEndOnEnd> g_GetEndOnEnd;

const char*
QueryGetEndOnEnd::getQueryName() const
{
  return "GETENDONEND";
}

void
QueryGetEndOnEnd::initialize()
{
  if (!_isInitialized)
  {
    QueryGetEndOnEndSQLStatement<QueryGetEndOnEnd> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETENDONEND");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetEndOnEnd::findEndOnEnd(std::vector<const tse::EndOnEnd*>& endOnEnds,
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

  tse::EndOnEnd* endOnEnd = nullptr;
  tse::EndOnEnd* endOnEndPrev = nullptr;
  while ((row = res.nextRow()))
  {
    endOnEnd = QueryGetEndOnEndSQLStatement<QueryGetEndOnEnd>::mapRowToEndOnEnd(row, endOnEndPrev);
    if (endOnEnd != endOnEndPrev)
      endOnEnds.push_back(endOnEnd);
    endOnEndPrev = endOnEnd;
  }
  LOG4CXX_INFO(_logger,
               "GETENDONEND: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                         << stopCPU() << ")");
  res.freeResult();
} // findEndOnEnd()

///////////////////////////////////////////////////////////
//  QueryGetEndOnEndHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetEndOnEndHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetEndOnEndHistorical"));
std::string QueryGetEndOnEndHistorical::_baseSQL;
bool QueryGetEndOnEndHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetEndOnEndHistorical> g_GetEndOnEndHistorical;

const char*
QueryGetEndOnEndHistorical::getQueryName() const
{
  return "GETENDONENDHISTORICAL";
}

void
QueryGetEndOnEndHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetEndOnEndHistoricalSQLStatement<QueryGetEndOnEndHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETENDONENDHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetEndOnEndHistorical::findEndOnEnd(std::vector<const tse::EndOnEnd*>& endOnEnds,
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

  tse::EndOnEnd* endOnEnd = nullptr;
  tse::EndOnEnd* endOnEndPrev = nullptr;
  while ((row = res.nextRow()))
  {
    endOnEnd = QueryGetEndOnEndHistoricalSQLStatement<QueryGetEndOnEndHistorical>::mapRowToEndOnEnd(
        row, endOnEndPrev);
    if (endOnEnd != endOnEndPrev)
      endOnEnds.push_back(endOnEnd);
    endOnEndPrev = endOnEnd;
  }
  LOG4CXX_INFO(_logger,
               "GETENDONENDHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                   << " (" << stopCPU() << ")");
  res.freeResult();
} // findEndOnEnd()
}
