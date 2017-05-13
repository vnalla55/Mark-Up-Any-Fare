//----------------------------------------------------------------------------
//  File:           QueryGetFareClassApp.cpp
//  Description:    QueryGetFareClassApp
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
#include "DBAccess/Queries/QueryGetFareClassApp.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareClassAppSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetFareClassApp::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetFareClassApp"));
std::string QueryGetFareClassApp::_baseSQL;
bool QueryGetFareClassApp::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFareClassApp> g_GetFareClassApp;

const char*
QueryGetFareClassApp::getQueryName() const
{
  return "GETFARECLASSAPP";
}

void
QueryGetFareClassApp::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareClassAppSQLStatement<QueryGetFareClassApp> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFARECLASSAPP");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetFareClassApp::findFareClassApp(std::vector<const tse::FareClassAppInfo*>& lstFCA,
                                       const VendorCode& vendor,
                                       const CarrierCode& carrier,
                                       const TariffNumber ruleTariff,
                                       const RuleNumber& rule,
                                       const FareClassCode& fareClass)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  char rtStr[15];
  sprintf(rtStr, "%d", ruleTariff);

  substParm(vendor, 1);
  substParm(carrier, 2);
  substParm(rtStr, 3);
  substParm(rule, 4);
  substParm(fareClass, 5);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::FareClassAppInfo* fcaPrev = nullptr;
  tse::FareClassAppInfo* fca = nullptr;
  while ((row = res.nextRow()))
  {
    fca = QueryGetFareClassAppSQLStatement<QueryGetFareClassApp>::mapRowToFareClassAppInfo(row,
                                                                                           fcaPrev);
    if (fca != fcaPrev)
      lstFCA.push_back(fca);

    fcaPrev = fca;
  }

  LOG4CXX_INFO(_logger,
               "GETFARECLASSAPP: NumRows: " << res.numRows() << " Time = " << stopTimer() << " ("
                                            << stopCPU() << ") mSecs");
  res.freeResult();
} // findFareClassApp()

///////////////////////////////////////////////////////////
//  QueryGetFareClassAppHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetFareClassAppHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetFareClassAppHistorical"));
std::string QueryGetFareClassAppHistorical::_baseSQL;
bool QueryGetFareClassAppHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFareClassAppHistorical> g_GetFareClassAppHistorical;

const char*
QueryGetFareClassAppHistorical::getQueryName() const
{
  return "GETFARECLASSAPPHISTORICAL";
}

void
QueryGetFareClassAppHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareClassAppHistoricalSQLStatement<QueryGetFareClassAppHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFARECLASSAPPHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetFareClassAppHistorical::findFareClassApp(std::vector<const tse::FareClassAppInfo*>& lstFCA,
                                                 const VendorCode& vendor,
                                                 const CarrierCode& carrier,
                                                 const TariffNumber ruleTariff,
                                                 const RuleNumber& rule,
                                                 const FareClassCode& fareClass,
                                                 const DateTime& startDate,
                                                 const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  char rtStr[15];
  sprintf(rtStr, "%d", ruleTariff);

  substParm(vendor, 1);
  substParm(carrier, 2);
  substParm(rtStr, 3);
  substParm(rule, 4);
  substParm(fareClass, 5);
  substParm(6, startDate);
  substParm(7, endDate);
  substParm(8, endDate);
  substParm(vendor, 9);
  substParm(carrier, 10);
  substParm(rtStr, 11);
  substParm(rule, 12);
  substParm(fareClass, 13);
  substParm(14, startDate);
  substParm(15, endDate);
  substParm(16, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::FareClassAppInfo* fcaPrev = nullptr;
  tse::FareClassAppInfo* fca = nullptr;
  while ((row = res.nextRow()))
  {
    fca = QueryGetFareClassAppHistoricalSQLStatement<
        QueryGetFareClassAppHistorical>::mapRowToFareClassAppInfo(row, fcaPrev);
    if (fca != fcaPrev)
      lstFCA.push_back(fca);

    fcaPrev = fca;
  }
  LOG4CXX_INFO(_logger,
               "GETFARECLASSAPPHISTORICAL: NumRows: " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ") mSecs");
  res.freeResult();
} // findFareClassApp()

///////////////////////////////////////////////////////////
//  QueryGetAllFareClassApp
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllFareClassApp::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllFareClassApp"));
std::string QueryGetAllFareClassApp::_baseSQL;
bool QueryGetAllFareClassApp::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllFareClassApp> g_GetAllFareClassApp;

const char*
QueryGetAllFareClassApp::getQueryName() const
{
  return "GETALLFARECLASSAPP";
}

void
QueryGetAllFareClassApp::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllFareClassAppSQLStatement<QueryGetAllFareClassApp> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLFARECLASSAPP");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
tse::QueryGetAllFareClassApp::findAllFareClassApp(std::vector<const tse::FareClassAppInfo*>& lstFCA)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::FareClassAppInfo* fcaPrev = nullptr;
  tse::FareClassAppInfo* fca = nullptr;
  while ((row = res.nextRow()))
  {
    fca = QueryGetAllFareClassAppSQLStatement<QueryGetAllFareClassApp>::mapRowToFareClassAppInfo(
        row, fcaPrev);
    if (fca != fcaPrev)
      lstFCA.push_back(fca);

    fcaPrev = fca;
  }

  LOG4CXX_INFO(_logger,
               "GETALLFARECLASSAPP: NumRows: " << res.numRows() << " Time = " << stopTimer() << " ("
                                               << stopCPU() << ") mSecs");
  res.freeResult();
}
}
