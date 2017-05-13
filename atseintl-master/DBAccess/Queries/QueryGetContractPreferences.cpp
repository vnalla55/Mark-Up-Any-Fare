//----------------------------------------------------------------------------
//  File:           QueryGetContractPreferences.cpp
//  Description:    QueryGetContractPreferences
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

#include "DBAccess/Queries/QueryGetContractPreferences.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetContractPreferencesSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetContractPreferences::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetContractPreferences"));
std::string QueryGetContractPreferences::_baseSQL;
bool QueryGetContractPreferences::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetContractPreferences> g_GetContractPreferences;

const char*
QueryGetContractPreferences::getQueryName() const
{
  return "GETCONTRACTPREFERENCES";
}

void
QueryGetContractPreferences::initialize()
{
  if (!_isInitialized)
  {
    QueryGetContractPreferencesSQLStatement<QueryGetContractPreferences> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCONTRACTPREFERENCES");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetContractPreferences::findContractPreferences(std::vector<ContractPreference*>& cpList,
                                                     const PseudoCityCode& pseudoCity,
                                                     const CarrierCode& carrier,
                                                     const RuleNumber& rule)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, pseudoCity);
  substParm(2, carrier);
  substParm(3, rule);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    cpList.push_back(QueryGetContractPreferencesSQLStatement<
        QueryGetContractPreferences>::mapRowToContractPreference(row));
  }
  LOG4CXX_INFO(_logger,
               "GETCONTRACTPREFERENCES: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                    << " (" << stopCPU() << ")");
  res.freeResult();
} // findContractPreferences()
///////////////////////////////////////////////////////////
//
//  QueryGetContractPreferencesHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetContractPreferencesHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetContractPreferencesHistorical"));
std::string QueryGetContractPreferencesHistorical::_baseSQL;
bool QueryGetContractPreferencesHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetContractPreferencesHistorical> g_GetContractPreferencesHistorical;

const char*
QueryGetContractPreferencesHistorical::getQueryName() const
{
  return "GETCONTRACTPREFERENCESHISTORICAL";
}

void
QueryGetContractPreferencesHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetContractPreferencesHistoricalSQLStatement<QueryGetContractPreferencesHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCONTRACTPREFERENCESHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetContractPreferencesHistorical::findContractPreferences(
    std::vector<ContractPreference*>& cpList,
    const PseudoCityCode& pseudoCity,
    const CarrierCode& carrier,
    const RuleNumber& rule)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, pseudoCity);
  substParm(2, carrier);
  substParm(3, rule);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    cpList.push_back(QueryGetContractPreferencesHistoricalSQLStatement<
        QueryGetContractPreferencesHistorical>::mapRowToContractPreference(row));
  }
  LOG4CXX_INFO(_logger,
               "GETCONTRACTPREFERENCESHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findContractPreferences()
///////////////////////////////////////////////////////////
//
//  QueryGetAllContractPreferencesHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllContractPreferencesHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.QueryGetAllContractPreferencesHistorical"));
std::string QueryGetAllContractPreferencesHistorical::_baseSQL;
bool QueryGetAllContractPreferencesHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllContractPreferencesHistorical>
g_GetAllContractPreferencesHistorical;

const char*
QueryGetAllContractPreferencesHistorical::getQueryName() const
{
  return "GETALLCONTRACTPREFERENCESHISTORICAL";
}

void
QueryGetAllContractPreferencesHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllContractPreferencesHistoricalSQLStatement<QueryGetAllContractPreferencesHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLCONTRACTPREFERENCESHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllContractPreferencesHistorical::findAllContractPreferences(
    std::vector<tse::ContractPreference*>& lstCP)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstCP.push_back(QueryGetAllContractPreferencesHistoricalSQLStatement<
        QueryGetAllContractPreferencesHistorical>::mapRowToContractPreference(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLCONTRACTPREFERENCESHISTORICAL: NumRows = " << res.numRows() << " Time = "
                                                                 << stopTimer() << " mSecs");
  res.freeResult();
} // findAllContractPreferences()
}
