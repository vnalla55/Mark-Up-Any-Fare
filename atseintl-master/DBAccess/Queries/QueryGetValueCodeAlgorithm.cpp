//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetValueCodeAlgorithm.h"

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetValueCodeAlgorithmSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetValueCodeAlgorithm::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetValueCodeAlgorithm"));
std::string QueryGetValueCodeAlgorithm::_baseSQL;
bool QueryGetValueCodeAlgorithm::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetValueCodeAlgorithm> g_GetValueCodeAlgorithm;

const char*
QueryGetValueCodeAlgorithm::getQueryName() const
{
  return "GETVALUECODEALGORITHM";
}

void
QueryGetValueCodeAlgorithm::initialize()
{
  if (!_isInitialized)
  {
    QueryGetValueCodeAlgorithmSQLStatement<QueryGetValueCodeAlgorithm> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETVALUECODEALGORITHM");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetValueCodeAlgorithm::findValueCodeAlgorithm(
    std::vector<ValueCodeAlgorithm*>& valueCodeAlgorithms,
    const VendorCode& vendor,
    const CarrierCode& carrier,
    const std::string& name)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(vendor, 1);
  substParm(carrier, 2);
  substParm(name, 3);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    valueCodeAlgorithms.push_back(QueryGetValueCodeAlgorithmSQLStatement<
        QueryGetValueCodeAlgorithm>::mapRowToValueCodeAlgorithm(row));
  }
  LOG4CXX_INFO(_logger,
               "GETVALUECODEALGORITHM: NumRows " << res.numRows() << " Time = " << stopTimer()
                                                 << " (" << stopCPU() << ") mSecs");
  res.freeResult();
}

///////////////////////////////////////////////////////////
//  QueryGetValueCodeAlgorithmHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetValueCodeAlgorithmHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetValueCodeAlgorithmHistorical"));
std::string QueryGetValueCodeAlgorithmHistorical::_baseSQL;
bool QueryGetValueCodeAlgorithmHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetValueCodeAlgorithmHistorical> g_GetValueCodeAlgorithmHistorical;

const char*
QueryGetValueCodeAlgorithmHistorical::getQueryName() const
{
  return "GETVALUECODEALGORITHMHISTORICAL";
}

void
QueryGetValueCodeAlgorithmHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetValueCodeAlgorithmHistoricalSQLStatement<QueryGetValueCodeAlgorithmHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETVALUECODEALGORITHMHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetValueCodeAlgorithmHistorical::findValueCodeAlgorithm(
    std::vector<ValueCodeAlgorithm*>& valueCodeAlgorithms,
    const VendorCode& vendor,
    const CarrierCode& carrier,
    const std::string& name,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(vendor, 1);
  substParm(carrier, 2);
  substParm(name, 3);
  substParm(4, startDate);
  substParm(5, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    valueCodeAlgorithms.push_back(QueryGetValueCodeAlgorithmHistoricalSQLStatement<
        QueryGetValueCodeAlgorithmHistorical>::mapRowToValueCodeAlgorithm(row));
  }
  LOG4CXX_INFO(_logger,
               "GETVALUECODEALGORITHMHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
}
} // tse
