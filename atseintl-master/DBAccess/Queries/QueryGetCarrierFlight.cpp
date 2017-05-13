//----------------------------------------------------------------------------
//  File:           QueryGetCarrierFlight.cpp
//  Description:    QueryGetCarrierFlight
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
#include "DBAccess/Queries/QueryGetCarrierFlight.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCarrierFlightSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetCarrierFlight::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetCarrierFlight"));
std::string QueryGetCarrierFlight::_baseSQL;
bool QueryGetCarrierFlight::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCarrierFlight> g_GetCarrierFlight;

const char*
QueryGetCarrierFlight::getQueryName() const
{
  return "GETCARRIERFLIGHT";
}

void
QueryGetCarrierFlight::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCarrierFlightSQLStatement<QueryGetCarrierFlight> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCARRIERFLIGHT");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetCarrierFlight::findCarrierFlight(std::vector<tse::CarrierFlight*>& lstCF,
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

  tse::CarrierFlight* cf = nullptr;
  tse::CarrierFlight* cfPrev = nullptr;
  while ((row = res.nextRow()))
  {
    cf = QueryGetCarrierFlightSQLStatement<QueryGetCarrierFlight>::mapRowToCarrierFlight(row,
                                                                                         cfPrev);
    if (cf != cfPrev)
      lstCF.push_back(cf);

    cfPrev = cf;
  }
  LOG4CXX_INFO(_logger,
               "GETCARRIERFLIGHT: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                              << stopCPU() << ")");
  res.freeResult();
} // findCarrierFlight()

int
QueryGetCarrierFlight::checkFlightWildCard(const char* fltStr)
{
  if (fltStr[0] == '*')
    return -1;
  else
    return stringToInteger(fltStr, __LINE__); // lint !e668
} // checkFlightWildCard()

int
QueryGetCarrierFlight::stringToInteger(const char* stringVal, int lineNumber)
{
  if (stringVal == nullptr)
  {
    LOG4CXX_FATAL(_logger, "stringToInteger - Null pointer to int data. Linber: " << lineNumber);
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
//  QueryGetCarrierFlightHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetCarrierFlightHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetCarrierFlightHistorical"));
std::string QueryGetCarrierFlightHistorical::_baseSQL;
bool QueryGetCarrierFlightHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCarrierFlightHistorical> g_GetCarrierFlightHistorical;

const char*
QueryGetCarrierFlightHistorical::getQueryName() const
{
  return "GETCARRIERFLIGHTHISTORICAL";
}

void
QueryGetCarrierFlightHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCarrierFlightHistoricalSQLStatement<QueryGetCarrierFlightHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCARRIERFLIGHTHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetCarrierFlightHistorical::findCarrierFlight(std::vector<tse::CarrierFlight*>& lstCF,
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

  tse::CarrierFlight* cf = nullptr;
  tse::CarrierFlight* cfPrev = nullptr;
  while ((row = res.nextRow()))
  {
    cf = QueryGetCarrierFlightHistoricalSQLStatement<
        QueryGetCarrierFlightHistorical>::mapRowToCarrierFlight(row, cfPrev);
    if (cf != cfPrev)
      lstCF.push_back(cf);

    cfPrev = cf;
  }
  LOG4CXX_INFO(_logger,
               "GETCARRIERFLIGHTHISTORICAL: NumRows = " << res.numRows() << " Time = "
                                                        << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findCarrierFlight()
}
