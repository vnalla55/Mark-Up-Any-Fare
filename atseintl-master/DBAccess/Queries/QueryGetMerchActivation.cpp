//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetMerchActivation.h"

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetMerchActivationSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetMerchActivation::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetMerchActivation"));
std::string QueryGetMerchActivation::_baseSQL;
bool QueryGetMerchActivation::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMerchActivation> g_GetMerchActivation;

const char*
QueryGetMerchActivation::getQueryName() const
{
  return "GETMERCHACTIVATION";
}

void
QueryGetMerchActivation::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMerchActivationSQLStatement<QueryGetMerchActivation> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMERCHACTIVATION");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetMerchActivation::findMerchActivationInfo(
    std::vector<MerchActivationInfo*>& merchActivations,
    int productId,
    const CarrierCode& carrier,
    const PseudoCityCode& pseudoCity)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, productId);
  substParm(carrier, 2);
  substParm(pseudoCity, 3);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    merchActivations.push_back(
        QueryGetMerchActivationSQLStatement<QueryGetMerchActivation>::mapRowToMerchActivationInfo(
            row));
  }
  LOG4CXX_INFO(_logger,
               "GETMERCHACTIVATION: NumRows " << res.numRows() << " Time = " << stopTimer() << " ("
                                              << stopCPU() << ") mSecs");
  res.freeResult();
}

///////////////////////////////////////////////////////////
//  QueryGetMerchActivationHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetMerchActivationHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetMerchActivationHistorical"));
std::string QueryGetMerchActivationHistorical::_baseSQL;
bool QueryGetMerchActivationHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMerchActivationHistorical> g_GetMerchActivationHistorical;

const char*
QueryGetMerchActivationHistorical::getQueryName() const
{
  return "GETMERCHACTIVATIONHISTORICAL";
}

void
QueryGetMerchActivationHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMerchActivationHistoricalSQLStatement<QueryGetMerchActivationHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMERCHACTIVATIONHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetMerchActivationHistorical::findMerchActivationInfo(
    std::vector<MerchActivationInfo*>& merchActivations,
    int productId,
    const CarrierCode& carrier,
    const PseudoCityCode& pseudoCity,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, productId);
  substParm(carrier, 2);
  substParm(pseudoCity, 3);
  substParm(4, startDate);
  substParm(5, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    merchActivations.push_back(QueryGetMerchActivationHistoricalSQLStatement<
        QueryGetMerchActivationHistorical>::mapRowToMerchActivationInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETMERCHACTIVATIONHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
}
} // tse
