//----------------------------------------------------------------------------
//  File:           QueryGetMiscFareTag.cpp
//  Description:    QueryGetMiscFareTag
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
#include "DBAccess/Queries/QueryGetMiscFareTag.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetMiscFareTagSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetMiscFareTag::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetMiscFareTag"));
std::string QueryGetMiscFareTag::_baseSQL;
bool QueryGetMiscFareTag::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMiscFareTag> g_GetMiscFareTag;

const char*
QueryGetMiscFareTag::getQueryName() const
{
  return "GETMISCFARETAG";
}

void
QueryGetMiscFareTag::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMiscFareTagSQLStatement<QueryGetMiscFareTag> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMISCFARETAG");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMiscFareTag::findMiscFareTag(std::vector<MiscFareTag*>& fareTags,
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
    fareTags.push_back(
        QueryGetMiscFareTagSQLStatement<QueryGetMiscFareTag>::mapRowToMiscFareTag(row));
  }
  LOG4CXX_INFO(_logger,
               "GETMISCFARETAG: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                            << stopCPU() << ")");
  res.freeResult();
} // findMiscFareTag()

///////////////////////////////////////////////////////////
//  QueryGetMiscFareTagHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetMiscFareTagHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetMiscFareTagHistorical"));
std::string QueryGetMiscFareTagHistorical::_baseSQL;
bool QueryGetMiscFareTagHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMiscFareTagHistorical> g_GetMiscFareTagHistorical;

const char*
QueryGetMiscFareTagHistorical::getQueryName() const
{
  return "GETMISCFARETAGHISTORICAL";
}

void
QueryGetMiscFareTagHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMiscFareTagHistoricalSQLStatement<QueryGetMiscFareTagHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMISCFARETAGHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMiscFareTagHistorical::findMiscFareTag(std::vector<MiscFareTag*>& fareTags,
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
    fareTags.push_back(QueryGetMiscFareTagHistoricalSQLStatement<
        QueryGetMiscFareTagHistorical>::mapRowToMiscFareTag(row));
  }
  LOG4CXX_INFO(_logger,
               "GETMISCFARETAGHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ")");
  res.freeResult();
} // findMiscFareTag()
}
