//----------------------------------------------------------------------------
//     � 2010, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetBaggageSectorCarrierApp.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetBaggageSectorCarrierAppSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetBaggageSectorCarrierApp::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetBaggageSectorCarrierApp"));
std::string QueryGetBaggageSectorCarrierApp::_baseSQL;
bool QueryGetBaggageSectorCarrierApp::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetBaggageSectorCarrierApp> g_GetBaggageSectorCarrierApp;

const char*
QueryGetBaggageSectorCarrierApp::getQueryName() const
{
  return "QUERYGETBAGGAGESECTORCARRIERAPP";
}

void
QueryGetBaggageSectorCarrierApp::initialize()
{
  if (!_isInitialized)
  {
    QueryGetBaggageSectorCarrierAppSQLStatement<QueryGetBaggageSectorCarrierApp> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("QUERYGETBAGGAGESECTORCARRIERAPP");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetBaggageSectorCarrierApp::findBaggageSectorCarrierApp(
    std::vector<tse::BaggageSectorCarrierApp*>& crxApp, const CarrierCode& carrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  substParm(1, carrier);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    crxApp.push_back(QueryGetBaggageSectorCarrierAppSQLStatement<
        QueryGetBaggageSectorCarrierApp>::mapRowToBaggageSectorCarrierApp(row));
  }
  LOG4CXX_INFO(_logger,
               "QUERYGETBAGGAGESECTORCARRIERAPP: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
}

///////////////////////////////////////////////////////////
//
//  QueryGetBaggageSectorCarrierAppHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetBaggageSectorCarrierAppHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.QueryGetBaggageSectorCarrierAppHistorical"));
std::string QueryGetBaggageSectorCarrierAppHistorical::_baseSQL;
bool QueryGetBaggageSectorCarrierAppHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetBaggageSectorCarrierAppHistorical>
g_GetBaggageSectorCarrierAppHistorical;

const char*
QueryGetBaggageSectorCarrierAppHistorical::getQueryName() const
{
  return "QUERYGETBAGGAGESECTORCARRIERAPPHISTORICAL";
}

void
QueryGetBaggageSectorCarrierAppHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetBaggageSectorCarrierAppHistoricalSQLStatement<QueryGetBaggageSectorCarrierAppHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("QUERYGETBAGGAGESECTORCARRIERAPPHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetBaggageSectorCarrierAppHistorical::findBaggageSectorCarrierApp(
    std::vector<tse::BaggageSectorCarrierApp*>& crxApp, const CarrierCode& carrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, carrier);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    crxApp.push_back(QueryGetBaggageSectorCarrierAppHistoricalSQLStatement<
        QueryGetBaggageSectorCarrierAppHistorical>::mapRowToBaggageSectorCarrierApp(row));
  }
  LOG4CXX_INFO(_logger,
               "QUERYGETBAGGAGESECTORCARRIERAPPHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
}

///////////////////////////////////////////////////////////
//
//  QueryGetAllBaggageSectorCarrierApp()
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllBaggageSectorCarrierApp::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetAllBaggageSectorCarrierApp"));
std::string QueryGetAllBaggageSectorCarrierApp::_baseSQL;
bool QueryGetAllBaggageSectorCarrierApp::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllBaggageSectorCarrierApp> g_GetAllBaggageSectorCarrierApp;

const char*
QueryGetAllBaggageSectorCarrierApp::getQueryName() const
{
  return "QUERYGETALLBAGGAGESECTORCARRIERAPP";
}
void
QueryGetAllBaggageSectorCarrierApp::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllBaggageSectorCarrierAppSQLStatement<QueryGetAllBaggageSectorCarrierApp> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("QUERYGETALLBAGGAGESECTORCARRIERAPP");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllBaggageSectorCarrierApp::findAllBaggageSectorCarrierApp(
    std::vector<tse::BaggageSectorCarrierApp*>& crxApp)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    crxApp.push_back(QueryGetAllBaggageSectorCarrierAppSQLStatement<
        QueryGetAllBaggageSectorCarrierApp>::mapRowToBaggageSectorCarrierApp(row));
  }
  LOG4CXX_INFO(_logger,
               "QUERYGETALLBAGGAGESECTORCARRIERAPP: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
}

///////////////////////////////////////////////////////////
//
//  QueryGetAllBaggageSectorCarrierAppHistorical()
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllBaggageSectorCarrierAppHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.QueryGetAllBaggageSectorCarrierAppHistorical"));
std::string QueryGetAllBaggageSectorCarrierAppHistorical::_baseSQL;
bool QueryGetAllBaggageSectorCarrierAppHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllBaggageSectorCarrierAppHistorical>
g_GetAllBaggageSectorCarrierAppHistorical;

const char*
QueryGetAllBaggageSectorCarrierAppHistorical::getQueryName() const
{
  return "QUERYGETALLBAGGAGESECTORCARRIERAPPHISTORICAL";
}

void
QueryGetAllBaggageSectorCarrierAppHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllBaggageSectorCarrierAppHistoricalSQLStatement<
        QueryGetAllBaggageSectorCarrierAppHistorical> sqlStatement;
    _baseSQL =
        sqlStatement.RegisterColumnsAndBaseSQL("QUERYGETALLBAGGAGESECTORCARRIERAPPHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetAllBaggageSectorCarrierAppHistorical::findAllBaggageSectorCarrierApp(
    std::vector<tse::BaggageSectorCarrierApp*>& crxApp)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    crxApp.push_back(QueryGetAllBaggageSectorCarrierAppHistoricalSQLStatement<
        QueryGetAllBaggageSectorCarrierAppHistorical>::mapRowToBaggageSectorCarrierApp(row));
  }
  LOG4CXX_INFO(_logger,
               "QUERYGETALLBAGGAGESECTORCARRIERAPPHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
}
}
