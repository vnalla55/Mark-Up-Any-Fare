//----------------------------------------------------------------------------
//  File:           QueryGetPFC.cpp
//  Description:    QueryGetPFC
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

#include "DBAccess/Queries/QueryGetPFC.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetPFCSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetPFC::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetPFC"));
std::string QueryGetPFC::_baseSQL;
bool QueryGetPFC::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetPFC> g_GetPFC;

const char*
QueryGetPFC::getQueryName() const
{
  return "GETPFC";
}

void
QueryGetPFC::initialize()
{
  if (!_isInitialized)
  {
    QueryGetPFCSQLStatement<QueryGetPFC> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETPFC");
    substTableDef(&_baseSQL);

    _isInitialized = true;
  }
} // initialize()

void
QueryGetPFC::findPfcPFC(std::vector<PfcPFC*>& lstPFC, const LocCode& loc)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, loc);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  PfcPFC* pfc = nullptr;
  PfcPFC* pfcPrev = nullptr;
  while ((row = res.nextRow()))
  {
    pfc = QueryGetPFCSQLStatement<QueryGetPFC>::mapRowToPfcPFC(row, pfcPrev);
    if (pfc != pfcPrev)
      lstPFC.push_back(pfc);

    pfcPrev = pfc;
  }
  LOG4CXX_INFO(_logger,
               "GETPFC: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                    << stopCPU() << ")");
  res.freeResult();
} // findPfcPFC()

int
QueryGetPFC::checkFlightWildCard(const char* fltStr)
{
  if (fltStr[0] == '*')
    return -1;
  else
    return stringToInteger(fltStr, __LINE__); // lint !e668
} // checkFlightWildCard()

int
QueryGetPFC::stringToInteger(const char* stringVal, int lineNumber)
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

log4cxx::LoggerPtr
QueryGetAllPFC::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllPFC"));
std::string QueryGetAllPFC::_baseSQL;
bool QueryGetAllPFC::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllPFC> g_GetAllPFC;

const char*
QueryGetAllPFC::getQueryName() const
{
  return "GETALLPFC";
}

void
QueryGetAllPFC::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllPFCSQLStatement<QueryGetAllPFC> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLPFC");
    substTableDef(&_baseSQL);

    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllPFC::findAllPfcPFC(std::vector<PfcPFC*>& lstPFC)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  PfcPFC* pfc = nullptr;
  PfcPFC* pfcPrev = nullptr;
  while ((row = res.nextRow()))
  {
    pfc = QueryGetAllPFCSQLStatement<QueryGetAllPFC>::mapRowToPfcPFC(row, pfcPrev);
    if (pfc != pfcPrev)
      lstPFC.push_back(pfc);

    pfcPrev = pfc;
  }
  LOG4CXX_INFO(_logger,
               "GETALLPFC: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                       << stopCPU() << ")");
  res.freeResult();
} // findAllPfcPFC()

log4cxx::LoggerPtr
QueryGetPFCHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetPFCHistorical"));
std::string QueryGetPFCHistorical::_baseSQL;
bool QueryGetPFCHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetPFCHistorical> g_GetPFCHistorical;

const char*
QueryGetPFCHistorical::getQueryName() const
{
  return "GETPFCHISTORICAL";
}

void
QueryGetPFCHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetPFCHistoricalSQLStatement<QueryGetPFCHistorical> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETPFCHISTORICAL");
    substTableDef(&_baseSQL);

    _isInitialized = true;
  }
} // initialize()

void
QueryGetPFCHistorical::findPfcPFC(std::vector<PfcPFC*>& lstPFC,
                                  const LocCode& loc,
                                  const DateTime& startDate,
                                  const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, loc);
  substParm(2, startDate);
  substParm(3, endDate);
  substParm(4, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  PfcPFC* pfc = nullptr;
  PfcPFC* pfcPrev = nullptr;
  while ((row = res.nextRow()))
  {
    pfc = QueryGetPFCHistoricalSQLStatement<QueryGetPFCHistorical>::mapRowToPfcPFC(row, pfcPrev);
    if (pfc != pfcPrev)
      lstPFC.push_back(pfc);

    pfcPrev = pfc;
  }
  LOG4CXX_INFO(_logger,
               "GETPFCHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                              << stopCPU() << ")");
  res.freeResult();
} // findPfcPFC()
}

