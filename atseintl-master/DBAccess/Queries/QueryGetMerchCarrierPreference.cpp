//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetMerchCarrierPreference.h"

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetMerchCarrierPreferenceSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetMerchCarrierPreference::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetMerchCarrierPreference"));
std::string QueryGetMerchCarrierPreference::_baseSQL;
bool QueryGetMerchCarrierPreference::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMerchCarrierPreference> g_GetMerchCarrierPreference;

const char*
QueryGetMerchCarrierPreference::getQueryName() const
{
  return "GETMERCHCARRIERPREFERENCE";
}

void
QueryGetMerchCarrierPreference::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMerchCarrierPreferenceSQLStatement<QueryGetMerchCarrierPreference> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMERCHCARRIERPREFERENCE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetMerchCarrierPreference::findMerchCarrierPreferenceInfo(
    std::vector<MerchCarrierPreferenceInfo*>& merchCarrierPreferences,
    const CarrierCode& carrier,
    const ServiceGroup& groupCode)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(carrier, 1);
  substParm(groupCode, 2);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    merchCarrierPreferences.push_back(QueryGetMerchCarrierPreferenceSQLStatement<
        QueryGetMerchCarrierPreference>::mapRowToMerchCarrierPreferenceInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETMERCHACARRIERPREFERENCE: NumRows " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ") mSecs");
  res.freeResult();
}

///////////////////////////////////////////////////////////
//  QueryGetMerchCarrierPreferenceHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetMerchCarrierPreferenceHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.QueryGetMerchCarrierPreferenceHistorical"));
std::string QueryGetMerchCarrierPreferenceHistorical::_baseSQL;
bool QueryGetMerchCarrierPreferenceHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMerchCarrierPreferenceHistorical>
g_GetMerchCarrierPreferenceHistorical;

const char*
QueryGetMerchCarrierPreferenceHistorical::getQueryName() const
{
  return "GETMERCHACARRIERPREFERENCEHISTORICAL";
}

void
QueryGetMerchCarrierPreferenceHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMerchCarrierPreferenceHistoricalSQLStatement<QueryGetMerchCarrierPreferenceHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMERCHACARRIERPREFERENCEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetMerchCarrierPreferenceHistorical::findMerchCarrierPreferenceInfo(
    std::vector<MerchCarrierPreferenceInfo*>& merchCarrierPreferences,
    const CarrierCode& carrier,
    const ServiceGroup& groupCode,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(carrier, 1);
  substParm(groupCode, 2);
  substParm(3, startDate);
  substParm(4, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    merchCarrierPreferences.push_back(QueryGetMerchCarrierPreferenceHistoricalSQLStatement<
        QueryGetMerchCarrierPreferenceHistorical>::mapRowToMerchCarrierPreferenceInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETMERCHACARRIERPREFERENCEHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
}
} // tse
