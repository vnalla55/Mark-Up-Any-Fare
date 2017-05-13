//-------------------------------------------------------------------------------
// (C) 2014, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc. Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetAirlineInterlineAgreement.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetAirlineInterlineAgreementSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetAirlineInterlineAgreement::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAirlineInterlineAgreement"));

std::string QueryGetAirlineInterlineAgreement::_baseSQL;
bool QueryGetAirlineInterlineAgreement::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAirlineInterlineAgreement> g_GetAirlineInterlineAgreement;

const char*
QueryGetAirlineInterlineAgreement::getQueryName() const
{
  return "GETAIRLINEINTERLINEAGREEMENT";
}

void
QueryGetAirlineInterlineAgreement::initialize()
{
  if (_isInitialized)
    return;

  QueryGetAirlineInterlineAgreementSQLStatement<QueryGetAirlineInterlineAgreement> sqlStatement;
  _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETAIRLINEINTERLINEAGREEMENT");
  substTableDef(&_baseSQL);
  _isInitialized = true;
}

void
QueryGetAirlineInterlineAgreement::findAirlineInterlineAgreement(
    std::vector<tse::AirlineInterlineAgreementInfo*>& aiaList,
    const NationCode& country,
    const CrsCode& gds,
    const CarrierCode& carrier)
{
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());

  substParm(1, country); // Insert country code (%1q) parameter into SQL query string.
  substParm(2, gds); // Insert GDS parameter (%2q) into SQL query string.
  substParm(3, carrier); // Insert carrier code (%3q) parameter into SQL query string.

  DBResultSet res(_dbAdapt);
  res.executeQuery(this);

  Row* row;
  AirlineInterlineAgreementInfo* aia;
  while ((row = res.nextRow()))
  {
    aia = QueryGetAirlineInterlineAgreementSQLStatement<
        QueryGetAirlineInterlineAgreement>::mapRowToAirlineInterlineAgreementInfo(row);
    aiaList.push_back(aia);
  }

  LOG4CXX_INFO(_logger,
               getQueryName() << ": NumRows = " << res.numRows() << " Time = " << stopTimer()
                              << " (" << stopCPU() << ")");
  res.freeResult();
}

const QueryGetAirlineInterlineAgreement&
QueryGetAirlineInterlineAgreement::
operator=(const QueryGetAirlineInterlineAgreement& rhs)
{
  if (this != &rhs)
  {
    *((SQLQuery*)this) = (SQLQuery&)rhs;
  }
  return *this;
}

const QueryGetAirlineInterlineAgreement&
QueryGetAirlineInterlineAgreement::
operator=(const std::string& rhs)
{
  if (this != &rhs)
  {
    *((SQLQuery*)this) = rhs;
  }
  return *this;
}

//-----------------------------------------------------------------------------
// HISTORICAL
//-----------------------------------------------------------------------------

log4cxx::LoggerPtr
QueryGetAirlineInterlineAgreementHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetAirlineInterlineAgreementHistorical"));

std::string QueryGetAirlineInterlineAgreementHistorical::_baseSQL;
bool QueryGetAirlineInterlineAgreementHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAirlineInterlineAgreementHistorical> g_GetAirlineInterlineAgreementHistorical;

const char*
QueryGetAirlineInterlineAgreementHistorical::getQueryName() const
{
  return "GETAIRLINEINTERLINEAGREEMENTHISTORICAL";
}

void
QueryGetAirlineInterlineAgreementHistorical::initialize()
{
  if (_isInitialized)
    return;

  QueryGetAirlineInterlineAgreementHistoricalSQLStatement<QueryGetAirlineInterlineAgreementHistorical> sqlStatement;
  _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETAIRLINEINTERLINEAGREEMENTHISTORICAL");
  substTableDef(&_baseSQL);
  _isInitialized = true;
}

void
QueryGetAirlineInterlineAgreementHistorical::findAirlineInterlineAgreement(
    std::vector<tse::AirlineInterlineAgreementInfo*>& aiaList,
    const NationCode& country,
    const CrsCode& gds,
    const CarrierCode& carrier,
    const DateTime& startDate,
    const DateTime& endDate)
{
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());

  substParm(1, country);
  substParm(2, gds);
  substParm(3, carrier);
  substParm(4, startDate);
  substParm(5, endDate);

  DBResultSet res(_dbAdapt);
  res.executeQuery(this);

  Row* row;
  AirlineInterlineAgreementInfo* aia;
  while ((row = res.nextRow()))
  {
    aia = QueryGetAirlineInterlineAgreementSQLStatement<
        QueryGetAirlineInterlineAgreementHistorical>::mapRowToAirlineInterlineAgreementInfo(row);
    aiaList.push_back(aia);
  }

  LOG4CXX_INFO(_logger,
               getQueryName() << ": NumRows = " << res.numRows() << " Time = " << stopTimer()
                              << " (" << stopCPU() << ")");
  res.freeResult();
}
}
