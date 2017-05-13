//----------------------------------------------------------------------------
//  File:           QueryGetBkgException.cpp
//  Description:    QueryGetBkgException
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
#include "DBAccess/Queries/QueryGetBkgException.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetBkgExceptionSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetBkgException::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetBkgException"));
std::string QueryGetBkgException::_baseSQL;
bool QueryGetBkgException::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetBkgException> g_GetBkgException;

const char*
QueryGetBkgException::getQueryName() const
{
  return "GETBKGEXCEPTION";
}

void
QueryGetBkgException::initialize()
{
  if (!_isInitialized)
  {
    QueryGetBkgExceptionSQLStatement<QueryGetBkgException> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETBKGEXCEPTION");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetBkgException::findBkgExcpt(std::vector<tse::BookingCodeExceptionSequence*>& bkgExcpts,
                                   const VendorCode& vendor,
                                   int itemNo)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char itemStr[20];
  sprintf(itemStr, "%d", itemNo);

  substParm(vendor, 1);
  substParm(itemStr, 2);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::BookingCodeExceptionSequence* bkgExcept = nullptr;
  tse::BookingCodeExceptionSequence* bkgExceptPrev = nullptr;
  while ((row = res.nextRow()))
  {
    bkgExcept =
        QueryGetBkgExceptionSQLStatement<QueryGetBkgException>::mapRowToBookingExceptionSequence(
            row, bkgExceptPrev);
    if (bkgExcept != bkgExceptPrev)
      bkgExcpts.push_back(bkgExcept);

    bkgExceptPrev = bkgExcept;
  }
  LOG4CXX_INFO(_logger,
               "GETBKGEXCEPTION: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                             << stopCPU() << ")");
  res.freeResult();
} // findBkgExcpt()

/*  convert two-digit year from alpha to int (xx -> 20nn). Missing year is returned as 0.
 *  TODO: Standardize all year fields to use this. Will require changes to seasonal appl,
 *        blackout dates and surcharges.
 */
int
QueryGetBkgException::mapYear(const char* s)
{
  int ret;
  if (s == nullptr || *s == '\0')
    return 0;
  ret = stringToInteger(s, __LINE__);
  if (0 <= ret && ret <= 99)
    ret += 2000;
  return ret;
} // mapYear()

int
QueryGetBkgException::stringToInteger(const char* stringVal, int lineNumber)
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
//  QueryGetBkgExceptionHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetBkgExceptionHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetBkgExceptionHistorical"));
std::string QueryGetBkgExceptionHistorical::_baseSQL;
bool QueryGetBkgExceptionHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetBkgExceptionHistorical> g_GetBkgExceptionHistorical;

const char*
QueryGetBkgExceptionHistorical::getQueryName() const
{
  return "GETBKGEXCEPTIONHISTORICAL";
}

void
QueryGetBkgExceptionHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetBkgExceptionHistoricalSQLStatement<QueryGetBkgExceptionHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETBKGEXCEPTIONHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetBkgExceptionHistorical::findBkgExcpt(
    std::vector<tse::BookingCodeExceptionSequence*>& bkgExcpts,
    const VendorCode& vendor,
    int itemNo,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char itemStr[20];
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

  tse::BookingCodeExceptionSequence* bkgExcept = nullptr;
  tse::BookingCodeExceptionSequence* bkgExceptPrev = nullptr;
  while ((row = res.nextRow()))
  {
    bkgExcept = QueryGetBkgExceptionHistoricalSQLStatement<
        QueryGetBkgExceptionHistorical>::mapRowToBookingExceptionSequence(row, bkgExceptPrev);
    if (bkgExcept != bkgExceptPrev)
      bkgExcpts.push_back(bkgExcept);

    bkgExceptPrev = bkgExcept;
  }
  LOG4CXX_INFO(_logger,
               "GETBKGEXCEPTIONHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                       << " (" << stopCPU() << ")");
  res.freeResult();
} // findBkgExcpt()
}
