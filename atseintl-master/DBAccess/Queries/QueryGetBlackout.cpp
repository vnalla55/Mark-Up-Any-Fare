//----------------------------------------------------------------------------
//  File:           QueryGetBlackout.cpp
//  Description:    QueryGetBlackout
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
#include "DBAccess/Queries/QueryGetBlackout.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetBlackoutSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetBlackout::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetBlackout"));
std::string QueryGetBlackout::_baseSQL;
bool QueryGetBlackout::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetBlackout> g_GetBlackout;

const char*
QueryGetBlackout::getQueryName() const
{
  return "GETBLACKOUT";
}

void
QueryGetBlackout::initialize()
{
  if (!_isInitialized)
  {
    QueryGetBlackoutSQLStatement<QueryGetBlackout> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETBLACKOUT");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetBlackout::findBlackoutInfo(std::vector<BlackoutInfo*>& blackout,
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
    blackout.push_back(QueryGetBlackoutSQLStatement<QueryGetBlackout>::mapRowToBlackoutInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETBLACKOUT: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                         << stopCPU() << ")");
  res.freeResult();
} // findBlackoutInfo()

int
QueryGetBlackout::stringToInteger(const char* stringVal, int lineNumber)
{
  if (stringVal == nullptr)
  {
    LOG4CXX_FATAL(_logger,
                  "stringToInteger - Null pointer to int data. LineNumber: " << lineNumber);
    throw std::runtime_error("Null pointer to int data");
  }
  else if (*stringVal == '-' || *stringVal == '+')
  {
    if (stringVal[1] < '0' || stringVal[1] > '9')
      return 0;
  }
  else if (*stringVal < '0' || *stringVal > '9')
  {
    return 0;
  }
  return atoi(stringVal);
} // stringToInteger()

///////////////////////////////////////////////////////////
//  QueryGetBlackoutHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetBlackoutHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetBlackoutHistorical"));
std::string QueryGetBlackoutHistorical::_baseSQL;
bool QueryGetBlackoutHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetBlackoutHistorical> g_GetBlackoutHistorical;

const char*
QueryGetBlackoutHistorical::getQueryName() const
{
  return "GETBLACKOUTHISTORICAL";
}

void
QueryGetBlackoutHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetBlackoutHistoricalSQLStatement<QueryGetBlackoutHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETBLACKOUTHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetBlackoutHistorical::findBlackoutInfo(std::vector<BlackoutInfo*>& blackout,
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
    blackout.push_back(
        QueryGetBlackoutHistoricalSQLStatement<QueryGetBlackoutHistorical>::mapRowToBlackoutInfo(
            row));
  }
  LOG4CXX_INFO(_logger,
               "GETBLACKOUTHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                   << " (" << stopCPU() << ")");
  res.freeResult();
} // findBlackoutInfo()
}
