//----------------------------------------------------------------------------
//  File:           QueryGetPfcAbsorb.cpp
//  Description:    QueryGetPfcAbsorb
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

#include "DBAccess/Queries/QueryGetPfcAbsorb.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetPfcAbsorbSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetPfcAbsorb::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetPfcAbsorb"));
std::string QueryGetPfcAbsorb::_baseSQL;
bool QueryGetPfcAbsorb::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetPfcAbsorb> g_GetPfcAbsorb;

const char*
QueryGetPfcAbsorb::getQueryName() const
{
  return "GETPFCABSORB";
}

void
QueryGetPfcAbsorb::initialize()
{
  if (!_isInitialized)
  {
    QueryGetPfcAbsorbSQLStatement<QueryGetPfcAbsorb> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETPFCABSORB");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetPfcAbsorb::findPfcAbsorb(std::vector<tse::PfcAbsorb*>& lstAbs,
                                 LocCode& pfcAirpt,
                                 CarrierCode& locCxr)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(pfcAirpt, 1);
  substParm(locCxr, 2);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());

  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstAbs.push_back(QueryGetPfcAbsorbSQLStatement<QueryGetPfcAbsorb>::mapRowToPfcAbsorb(row));
  }
  LOG4CXX_INFO(_logger,
               "GETPFCABSORB: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                          << stopCPU() << ")");
  res.freeResult();
} // findPfcAbsorb()

int
QueryGetPfcAbsorb::checkFlightWildCard(const char* fltStr)
{
  if (UNLIKELY(fltStr[0] == '*'))
    return -1;
  else
    return stringToInteger(fltStr, __LINE__); // lint !e668
} // checkFlightWildCard()

int
QueryGetPfcAbsorb::stringToInteger(const char* stringVal, int lineNumber)
{
  if (UNLIKELY(stringVal == nullptr))
  {
    LOG4CXX_FATAL(_logger,
                  "stringToInteger - Null pointer to int data. LineNumber: " << lineNumber);
    throw std::runtime_error("Null pointer to int data");
  }
  else if (UNLIKELY(*stringVal == '-' || *stringVal == '+'))
  {
    if (stringVal[1] < '0' || stringVal[1] > '9')
      return 0;
  }
  else if (LIKELY(*stringVal < '0' || *stringVal > '9'))
  {
    return 0;
  }
  return atoi(stringVal);
} // stringToInteger()

///////////////////////////////////////////////////////////
//
//  QueryGetAllPfcAbsorb
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllPfcAbsorb::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetAllPfcAbsorb"));
std::string QueryGetAllPfcAbsorb::_baseSQL;
bool QueryGetAllPfcAbsorb::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllPfcAbsorb> g_QueryGetAllPfcAbsorb;

const char*
QueryGetAllPfcAbsorb::getQueryName() const
{
  return "GETALLPFCABSORB";
}

void
QueryGetAllPfcAbsorb::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllPfcAbsorbSQLStatement<QueryGetAllPfcAbsorb> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLPFCABSORB");
    substTableDef(&_baseSQL);

    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllPfcAbsorb::findAllPfcAbsorb(std::vector<tse::PfcAbsorb*>& lstAbsorb)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    lstAbsorb.push_back(QueryGetPfcAbsorbSQLStatement<QueryGetPfcAbsorb>::mapRowToPfcAbsorb(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLPFCABSORB: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                             << stopCPU() << ")");
  res.freeResult();
} // findAllPfcAbsorb()

///////////////////////////////////////////////////////////
//
//  QueryGetPfcAbsorbHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetPfcAbsorbHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetPfcAbsorbHistorical"));
std::string QueryGetPfcAbsorbHistorical::_baseSQL;
bool QueryGetPfcAbsorbHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetPfcAbsorbHistorical> g_GetPfcAbsorbHistorical;

const char*
QueryGetPfcAbsorbHistorical::getQueryName() const
{
  return "GETPFCABSORBHISTORICAL";
}

void
QueryGetPfcAbsorbHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetPfcAbsorbHistoricalSQLStatement<QueryGetPfcAbsorbHistorical> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETPFCABSORBHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetPfcAbsorbHistorical::findPfcAbsorbHistorical(std::vector<tse::PfcAbsorb*>& lstAbs,
                                                     LocCode& pfcAirpt,
                                                     CarrierCode& locCxr,
                                                     const DateTime& startDate,
                                                     const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(pfcAirpt, 1);
  substParm(locCxr, 2);
  substParm(3, startDate);
  substParm(4, endDate);
  substParm(5, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstAbs.push_back(
        QueryGetPfcAbsorbHistoricalSQLStatement<QueryGetPfcAbsorbHistorical>::mapRowToPfcAbsorb(
            row));
  }
  LOG4CXX_INFO(_logger,
               "GETPFCABSORBHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                    << " (" << stopCPU() << ")");
  res.freeResult();
} // findPfcAborbHistorical()
}
