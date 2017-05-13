//----------------------------------------------------------------------------
//  File:           QueryGetTaxSegAbsorb.cpp
//  Description:    QueryGetTaxSegAbsorb
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

#include "DBAccess/Queries/QueryGetTaxSegAbsorb.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetTaxSegAbsorbSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetTaxSegAbsorb::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTaxSegAbsorb"));
std::string QueryGetTaxSegAbsorb::_baseSQL;
bool QueryGetTaxSegAbsorb::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxSegAbsorb> g_GetTaxSegAbsorb;

const char*
QueryGetTaxSegAbsorb::getQueryName() const
{
  return "GETTAXSEGABSORB";
}

void
QueryGetTaxSegAbsorb::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxSegAbsorbSQLStatement<QueryGetTaxSegAbsorb> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXSEGABSORB");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxSegAbsorb::findTaxSegAbsorb(std::vector<tse::TaxSegAbsorb*>& lstTSA,
                                       const CarrierCode& cxr)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, cxr);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()) != nullptr)
  {
    lstTSA.push_back(
        QueryGetTaxSegAbsorbSQLStatement<QueryGetTaxSegAbsorb>::mapRowToTaxSegAbsorb(row));
  }
  LOG4CXX_INFO(_logger,
               "GETTAXSEGABSORB: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                             << stopCPU() << ") mSecs");
  res.freeResult();
} // findTaxSegAbsorb()

int
QueryGetTaxSegAbsorb::checkFlightWildCard(const char* fltStr)
{
  if (fltStr[0] == '*')
    return -1;
  else
    return stringToInteger(fltStr, __LINE__); // lint !e668
} // checkFlightWildCard()

int
QueryGetTaxSegAbsorb::stringToInteger(const char* stringVal, int lineNumber)
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
//  QueryGetAllTaxSegAbsorb
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllTaxSegAbsorb::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllTaxSegAbsorb"));
std::string QueryGetAllTaxSegAbsorb::_baseSQL;
bool QueryGetAllTaxSegAbsorb::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllTaxSegAbsorb> g_GetAllTaxSegAbsorb;

const char*
QueryGetAllTaxSegAbsorb::getQueryName() const
{
  return "GETALLTAXSEGABSORB";
}

void
QueryGetAllTaxSegAbsorb::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllTaxSegAbsorbSQLStatement<QueryGetAllTaxSegAbsorb> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLTAXSEGABSORB");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllTaxSegAbsorb::findAllTaxSegAbsorb(std::vector<tse::TaxSegAbsorb*>& lstTSA)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()) != nullptr)
  {
    lstTSA.push_back(
        QueryGetAllTaxSegAbsorbSQLStatement<QueryGetAllTaxSegAbsorb>::mapRowToTaxSegAbsorb(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLTAXSEGABSORB: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                << " (" << stopCPU() << ") mSecs");
  res.freeResult();
} // findAllTaxSegAbsorb()

///////////////////////////////////////////////////////////
//
//  QueryGetTaxSegAbsorbHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetTaxSegAbsorbHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTaxSegAbsorbHistorical"));
std::string QueryGetTaxSegAbsorbHistorical::_baseSQL;
bool QueryGetTaxSegAbsorbHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxSegAbsorbHistorical> g_GetTaxSegAbsorbHistorical;

const char*
QueryGetTaxSegAbsorbHistorical::getQueryName() const
{
  return "GETTAXSEGABSORBHISTORICAL";
}

void
QueryGetTaxSegAbsorbHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxSegAbsorbHistoricalSQLStatement<QueryGetTaxSegAbsorbHistorical> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXSEGABSORBHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxSegAbsorbHistorical::findTaxSegAbsorb(std::vector<tse::TaxSegAbsorb*>& lstTSA,
                                                 const CarrierCode& cxr,
                                                 const DateTime& startDate,
                                                 const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, cxr);
  substParm(2, startDate);
  substParm(3, endDate);
  substParm(4, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()) != nullptr)
  {
    lstTSA.push_back(QueryGetTaxSegAbsorbHistoricalSQLStatement<
        QueryGetTaxSegAbsorbHistorical>::mapRowToTaxSegAbsorb(row));
  }
  LOG4CXX_INFO(_logger,
               "GETTAXSEGABSORBHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                       << " (" << stopCPU() << ") mSecs");
  res.freeResult();
} // findTaxSegAbsorb()
}
