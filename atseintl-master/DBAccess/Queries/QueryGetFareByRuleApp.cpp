//----------------------------------------------------------------------------
//  File:           QueryGetFareByRuleApp.cpp
//  Description:    QueryGetFareByRuleApp
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
#include "DBAccess/Queries/QueryGetFareByRuleApp.h"

#include "Common/Global.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareByRuleAppSQLStatement.h"

namespace tse
{
///////////////////////////////////////////////////////////
//  QueryGetFareByRuleApp
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetFareByRuleApp::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetFareByRuleApp"));
std::string QueryGetFareByRuleApp::_baseSQL;
bool QueryGetFareByRuleApp::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFareByRuleApp> g_GetFareByRuleApp;

const char*
QueryGetFareByRuleApp::getQueryName() const
{
  return "GETFAREBYRULEAPP";
}

void
QueryGetFareByRuleApp::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareByRuleAppSQLStatement<QueryGetFareByRuleApp> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFAREBYRULEAPP");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetFareByRuleApp::findFareByRuleApp(std::vector<FareByRuleApp*>& fbrApps,
                                         const CarrierCode& carrier,
                                         const AccountCode& accountCode)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, carrier);
  substParm(2, accountCode);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareByRuleApp* fbrPrev = nullptr;
  while ((row = res.nextRow()))
  {
    FareByRuleApp* fbr =
        QueryGetFareByRuleAppSQLStatement<QueryGetFareByRuleApp>::mapRowToFareByRuleApp(row,
                                                                                        fbrPrev);
    if (fbr != fbrPrev)
    {
      fbrApps.push_back(fbr);
      fbrPrev = fbr;
    }
  }
  LOG4CXX_INFO(_logger,
               "GETFAREBYRULEAPP: NumRows " << res.numRows() << " Time = " << stopTimer() << " ("
                                            << stopCPU() << ") mSecs");
  res.freeResult();
} // QueryGetFareByRuleApp::findFareByRuleApp()

///////////////////////////////////////////////////////////
//  QueryGetFareByRuleAppHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetFareByRuleAppHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetFareByRuleAppHistorical"));
std::string QueryGetFareByRuleAppHistorical::_baseSQL;
bool QueryGetFareByRuleAppHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFareByRuleAppHistorical> g_GetFareByRuleAppHistorical;

const char*
QueryGetFareByRuleAppHistorical::getQueryName() const
{
  return "GETFAREBYRULEAPPHISTORICAL";
}

void
QueryGetFareByRuleAppHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareByRuleAppHistoricalSQLStatement<QueryGetFareByRuleAppHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFAREBYRULEAPPHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetFareByRuleAppHistorical::findFareByRuleApp(std::vector<FareByRuleApp*>& fbrApps,
                                                   const CarrierCode& carrier,
                                                   const AccountCode& accountCode,
                                                   const DateTime& startDate,
                                                   const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, carrier);
  substParm(2, accountCode);
  substParm(3, startDate);
  substParm(4, endDate);
  substParm(5, carrier);
  substParm(6, accountCode);
  substParm(7, startDate);
  substParm(8, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareByRuleApp* fbrPrev = nullptr;
  while ((row = res.nextRow()))
  {
    FareByRuleApp* fbr = QueryGetFareByRuleAppHistoricalSQLStatement<
        QueryGetFareByRuleAppHistorical>::mapRowToFareByRuleApp(row, fbrPrev);
    if (fbr != fbrPrev)
    {
      fbrApps.push_back(fbr);
      fbrPrev = fbr;
    }
  }
  LOG4CXX_INFO(_logger,
               "GETFAREBYRULEAPPHISTORICAL: NumRows " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ") mSecs");
  res.freeResult();
} // QueryGetFareByRuleAppHistorical::findFareByRuleApp()

///////////////////////////////////////////////////////////
//  QueryGetFareByRuleAppRuleTariff
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetFareByRuleAppRuleTariff::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetFareByRuleAppRuleTariff"));
std::string QueryGetFareByRuleAppRuleTariff::_baseSQL;
bool QueryGetFareByRuleAppRuleTariff::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFareByRuleAppRuleTariff> g_GetFareByRuleAppRuleTariff;

const char*
QueryGetFareByRuleAppRuleTariff::getQueryName() const
{
  return "GETFAREBYRULEAPPRULETARIFF";
}

void
QueryGetFareByRuleAppRuleTariff::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareByRuleAppRuleTariffSQLStatement<QueryGetFareByRuleAppRuleTariff> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFAREBYRULEAPPRULETARIFF");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetFareByRuleAppRuleTariff::findFareByRuleApp(std::vector<FareByRuleApp*>& fbrApps,
                                                   const CarrierCode& carrier,
                                                   const TariffNumber& ruleTariff)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, carrier);
  substParm(2, ruleTariff);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareByRuleApp* fbrPrev = nullptr;
  while ((row = res.nextRow()))
  {
    FareByRuleApp* fbr = QueryGetFareByRuleAppRuleTariffSQLStatement<
        QueryGetFareByRuleAppRuleTariff>::mapRowToFareByRuleApp(row, fbrPrev);
    if (fbr != fbrPrev)
    {
      fbrApps.push_back(fbr);
      fbrPrev = fbr;
    }
  }
  LOG4CXX_INFO(_logger,
               "GETFAREBYRULEAPPRULETARIFF: NumRows " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ") mSecs");
  res.freeResult();
} // QueryGetFareByRuleAppRuleTariff::findFareByRuleApp()

///////////////////////////////////////////////////////////
//  QueryGetFareByRuleAppRuleTariffHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetFareByRuleAppRuleTariffHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetFareByRuleAppRuleTariffHistorical"));
std::string QueryGetFareByRuleAppRuleTariffHistorical::_baseSQL;
bool QueryGetFareByRuleAppRuleTariffHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFareByRuleAppRuleTariffHistorical>
g_GetFareByRuleAppRuleTariffHistorical;

const char*
QueryGetFareByRuleAppRuleTariffHistorical::getQueryName() const
{
  return "GETFAREBYRULEAPPRULETARIFFHISTORICAL";
}

void
QueryGetFareByRuleAppRuleTariffHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareByRuleAppRuleTariffHistoricalSQLStatement<QueryGetFareByRuleAppRuleTariffHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFAREBYRULEAPPRULETARIFFHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetFareByRuleAppRuleTariffHistorical::findFareByRuleApp(std::vector<FareByRuleApp*>& fbrApps,
                                                             const CarrierCode& carrier,
                                                             const TariffNumber& ruleTariff,
                                                             const DateTime& startDate,
                                                             const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, carrier);
  substParm(2, ruleTariff);
  substParm(3, startDate);
  substParm(4, endDate);
  substParm(5, carrier);
  substParm(6, ruleTariff);
  substParm(7, startDate);
  substParm(8, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareByRuleApp* fbrPrev = nullptr;
  while ((row = res.nextRow()))
  {
    FareByRuleApp* fbr = QueryGetFareByRuleAppRuleTariffHistoricalSQLStatement<
        QueryGetFareByRuleAppRuleTariffHistorical>::mapRowToFareByRuleApp(row, fbrPrev);
    if (fbr != fbrPrev)
    {
      fbrApps.push_back(fbr);
      fbrPrev = fbr;
    }
  }
  LOG4CXX_INFO(_logger,
               "GETFAREBYRULEAPPRULETARIFFHISTORICAL: NumRows "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ") mSecs");
  res.freeResult();
} // QueryGetFareByRuleAppRuleTariffHistorical::findFareByRuleApp()
} // tse
