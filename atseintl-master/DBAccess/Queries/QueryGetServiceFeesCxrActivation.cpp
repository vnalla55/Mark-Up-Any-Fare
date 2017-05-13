//----------------------------------------------------------------------------
//  2009, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetServiceFeesCxrActivation.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetServiceFeesCxrActivationSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetServiceFeesCxrActivation::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetServiceFeesCxrActivation"));
std::string QueryGetServiceFeesCxrActivation::_baseSQL;
bool QueryGetServiceFeesCxrActivation::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetServiceFeesCxrActivation> g_GetServiceFeesCxrActivation;

const char*
QueryGetServiceFeesCxrActivation::getQueryName() const
{
  return "GETSERVICEFEESCXRACTIVATION";
}

void
QueryGetServiceFeesCxrActivation::initialize()
{
  if (!_isInitialized)
  {
    QueryGetServiceFeesCxrActivationSQLStatement<QueryGetServiceFeesCxrActivation> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSERVICEFEESCXRACTIVATION");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetServiceFeesCxrActivation::findServiceFeesCxrActivation(
    std::vector<tse::ServiceFeesCxrActivation*>& svcCxr, const CarrierCode& carrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(carrier, 1);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    svcCxr.push_back(QueryGetServiceFeesCxrActivationSQLStatement<
        QueryGetServiceFeesCxrActivation>::mapRowToServiceFeesCxrActivation(row));
  }
  LOG4CXX_INFO(_logger,
               "GETTICKETINGFEES: NumRows " << res.numRows() << " Time = " << stopTimer() << " ("
                                            << stopCPU() << ") mSecs");
  res.freeResult();
}

///////////////////////////////////////////////////////////
//  QueryGetServiceFeesCxrActivationHistorical
///////////////////////////////////////////////////////////

log4cxx::LoggerPtr
QueryGetServiceFeesCxrActivationHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.QueryGetServiceFeesCxrActivationHistorical"));
std::string QueryGetServiceFeesCxrActivationHistorical::_baseSQL;
bool QueryGetServiceFeesCxrActivationHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetServiceFeesCxrActivationHistorical>
g_GetServiceFeesCxrActivationHistorical;

const char*
QueryGetServiceFeesCxrActivationHistorical::getQueryName() const
{
  return "GETSERVICEFEESCXRACTIVATIONHISTORICAL";
}

void
QueryGetServiceFeesCxrActivationHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetServiceFeesCxrActivationHistoricalSQLStatement<
        QueryGetServiceFeesCxrActivationHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSERVICEFEESCXRACTIVATIONHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetServiceFeesCxrActivationHistorical::findServiceFeesCxrActivation(
    std::vector<tse::ServiceFeesCxrActivation*>& svcCxr,
    const CarrierCode& carrier,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  substParm(carrier, 1);
  substParm(2, startDate);
  substParm(3, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    svcCxr.push_back(QueryGetServiceFeesCxrActivationHistoricalSQLStatement<
        QueryGetServiceFeesCxrActivationHistorical>::mapRowToServiceFeesCxrActivation(row));
  }
  LOG4CXX_INFO(_logger,
               "GETSERVICEFEESCXRACTIVATIONHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
}
}
