//----------------------------------------------------------------------------
//  File:           QueryGetPfcEssAirSvc.cpp
//  Description:    QueryGetPfcEssAirSvc
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

#include "DBAccess/Queries/QueryGetPfcEssAirSvc.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetPfcEssAirSvcSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetPfcEssAirSvc::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetPfcEssAirSvc"));
std::string QueryGetPfcEssAirSvc::_baseSQL;
bool QueryGetPfcEssAirSvc::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetPfcEssAirSvc> g_GetPfcEssAirSvc;

const char*
QueryGetPfcEssAirSvc::getQueryName() const
{
  return "GETPFCESSAIRSVC";
}

void
QueryGetPfcEssAirSvc::initialize()
{
  if (!_isInitialized)
  {
    QueryGetPfcEssAirSvcSQLStatement<QueryGetPfcEssAirSvc> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETPFCESSAIRSVC");
    substTableDef(&_baseSQL);

    _isInitialized = true;
  }
} // initialize()

void
QueryGetPfcEssAirSvc::findPfcEssAirSvc(std::vector<tse::PfcEssAirSvc*>& lstEAS,
                                       const LocCode& easHubArpt,
                                       const LocCode& easArpt)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, easHubArpt);
  substParm(2, easArpt);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::PfcEssAirSvc* eas = nullptr;
  tse::PfcEssAirSvc* easPrev = nullptr;
  while ((row = res.nextRow()))
  {
    eas =
        QueryGetPfcEssAirSvcSQLStatement<QueryGetPfcEssAirSvc>::mapRowToPfcEssAirSvc(row, easPrev);
    if (eas != easPrev)
      lstEAS.push_back(eas);

    easPrev = eas;
  }
  LOG4CXX_INFO(_logger,
               "GETPFCESSAIRSVC: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                             << stopCPU() << ")");
  res.freeResult();
} // findPfcEssAirSvc()
int
QueryGetPfcEssAirSvc::checkFlightWildCard(const char* fltStr)
{
  if (fltStr[0] == '*')
    return -1;
  else
    return stringToInteger(fltStr, __LINE__); // lint !e668
} // checkFlightWildCard()

int
QueryGetPfcEssAirSvc::stringToInteger(const char* stringVal, int lineNumber)
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
//
//  QueryGetAllPfcEssAirSvc
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllPfcEssAirSvc::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllPfcEssAirSvc"));
std::string QueryGetAllPfcEssAirSvc::_baseSQL;
bool QueryGetAllPfcEssAirSvc::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllPfcEssAirSvc> g_GetAllPfcEssAirSvc;

const char*
QueryGetAllPfcEssAirSvc::getQueryName() const
{
  return "GETALLPFCESSAIRSVC";
}

void
QueryGetAllPfcEssAirSvc::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllPfcEssAirSvcSQLStatement<QueryGetAllPfcEssAirSvc> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLPFCESSAIRSVC");
    substTableDef(&_baseSQL);

    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllPfcEssAirSvc::findAllPfcEssAirSvc(std::vector<tse::PfcEssAirSvc*>& lstEAS)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::PfcEssAirSvc* eas = nullptr;
  tse::PfcEssAirSvc* easPrev = nullptr;
  while ((row = res.nextRow()))
  {
    eas = QueryGetAllPfcEssAirSvcSQLStatement<QueryGetAllPfcEssAirSvc>::mapRowToPfcEssAirSvc(
        row, easPrev);
    if (eas != easPrev)
      lstEAS.push_back(eas);

    easPrev = eas;
  }
  LOG4CXX_INFO(_logger,
               "GETALLPFCESSAIRSVC: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllPfcEssAirSvc()

///////////////////////////////////////////////////////////
//
//  QueryGetPfcEssAirSvcHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetPfcEssAirSvcHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetPfcEssAirSvcHistorical"));
std::string QueryGetPfcEssAirSvcHistorical::_baseSQL;
bool QueryGetPfcEssAirSvcHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetPfcEssAirSvcHistorical> g_GetPfcEssAirSvcHistorical;

const char*
QueryGetPfcEssAirSvcHistorical::getQueryName() const
{
  return "GETPFCESSAIRSVCHISTORICAL";
}

void
QueryGetPfcEssAirSvcHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetPfcEssAirSvcHistoricalSQLStatement<QueryGetPfcEssAirSvcHistorical> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETPFCESSAIRSVCHISTORICAL");
    substTableDef(&_baseSQL);

    _isInitialized = true;
  }
} // initialize()

void
QueryGetPfcEssAirSvcHistorical::findPfcEssAirSvc(std::vector<tse::PfcEssAirSvc*>& lstEAS,
                                                 const LocCode& easHubArpt,
                                                 const LocCode& easArpt,
                                                 const DateTime& startDate,
                                                 const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, easHubArpt);
  substParm(2, easArpt);
  substParm(3, startDate);
  substParm(4, endDate);
  substParm(5, endDate);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::PfcEssAirSvc* eas = nullptr;
  tse::PfcEssAirSvc* easPrev = nullptr;
  while ((row = res.nextRow()))
  {
    eas = QueryGetPfcEssAirSvcHistoricalSQLStatement<
        QueryGetPfcEssAirSvcHistorical>::mapRowToPfcEssAirSvc(row, easPrev);
    if (eas != easPrev)
      lstEAS.push_back(eas);

    easPrev = eas;
  }
  LOG4CXX_INFO(_logger,
               "GETPFCESSAIRSVCHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                       << " (" << stopCPU() << ")");
  res.freeResult();
} // findPfcEssAirSvc()
}
