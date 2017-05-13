//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetOptionalServicesConcur.h"

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetOptionalServicesConcurSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetOptionalServicesConcur::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetOptionalServicesConcur"));
std::string QueryGetOptionalServicesConcur::_baseSQL;
bool QueryGetOptionalServicesConcur::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetOptionalServicesConcur> g_GetOptionalServicesConcur;

const char*
QueryGetOptionalServicesConcur::getQueryName() const
{
  return "GETOPTIONALSERVICESCONCUR";
}

void
QueryGetOptionalServicesConcur::initialize()
{
  if (!_isInitialized)
  {
    QueryGetOptionalServicesConcurSQLStatement<QueryGetOptionalServicesConcur> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETOPTIONALSERVICESCONCUR");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetOptionalServicesConcur::findOptionalServicesConcur(
    std::vector<OptionalServicesConcur*>& concurs,
    const VendorCode& vendor,
    const CarrierCode& carrier,
    const ServiceTypeCode& serviceTypeCode)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(vendor, 1);
  substParm(carrier, 2);
  substParm(serviceTypeCode, 3);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    concurs.push_back(QueryGetOptionalServicesConcurSQLStatement<
        QueryGetOptionalServicesConcur>::mapRowToOptionalServicesConcur(row));
  }
  LOG4CXX_INFO(_logger,
               "GETOPTIONALSERVICESCONCUR: NumRows " << res.numRows() << " Time = " << stopTimer()
                                                     << " (" << stopCPU() << ") mSecs");
  res.freeResult();
}

///////////////////////////////////////////////////////////
//  QueryGetOptionalServicesConcurHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetOptionalServicesConcurHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.QueryGetOptionalServicesConcurHistorical"));
std::string QueryGetOptionalServicesConcurHistorical::_baseSQL;
bool QueryGetOptionalServicesConcurHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetOptionalServicesConcurHistorical>
g_GetOptionalServicesConcurHistorical;

const char*
QueryGetOptionalServicesConcurHistorical::getQueryName() const
{
  return "GETOPTIONALSERVICESCONCURHISTORICAL";
}

void
QueryGetOptionalServicesConcurHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetOptionalServicesConcurHistoricalSQLStatement<QueryGetOptionalServicesConcurHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETOPTIONALSERVICESCONCURHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetOptionalServicesConcurHistorical::findOptionalServicesConcur(
    std::vector<OptionalServicesConcur*>& concurs,
    const VendorCode& vendor,
    const CarrierCode& carrier,
    const ServiceTypeCode& serviceTypeCode,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(vendor, 1);
  substParm(carrier, 2);
  substParm(serviceTypeCode, 3);
  substParm(4, startDate);
  substParm(5, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    concurs.push_back(QueryGetOptionalServicesConcurHistoricalSQLStatement<
        QueryGetOptionalServicesConcurHistorical>::mapRowToOptionalServicesConcur(row));
  }
  LOG4CXX_INFO(_logger,
               "GETOPTIONALSERVICESCONCURHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
}
} // tse
