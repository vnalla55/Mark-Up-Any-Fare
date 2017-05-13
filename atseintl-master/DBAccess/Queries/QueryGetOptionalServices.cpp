//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetOptionalServices.h"

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetOptionalServicesSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetOptionalServices::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetOptionalServices"));
std::string QueryGetOptionalServices::_baseSQL;
bool QueryGetOptionalServices::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetOptionalServices> g_GetOptionalServices;

const char*
QueryGetOptionalServices::getQueryName() const
{
  return "GETOPTIONALSERVICES";
}

void
QueryGetOptionalServices::initialize()
{
  if (!_isInitialized)
  {
    QueryGetOptionalServicesSQLStatement<QueryGetOptionalServices> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETOPTIONALSERVICES");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetOptionalServices::findOptionalServicesInfo(
    std::vector<OptionalServicesInfo*>& optionalServices,
    const VendorCode& vendor,
    const CarrierCode& carrier,
    const Indicator& fltTktMerchInd)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(vendor, 1);
  substParm(carrier, 2);
  substParm(3, fltTktMerchInd);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  OptionalServicesInfo* service = nullptr;
  OptionalServicesInfo* servicePrev = nullptr;
  while ((row = res.nextRow()))
  {
    service = QueryGetOptionalServicesSQLStatement<
        QueryGetOptionalServices>::mapRowToOptionalServicesInfo(row, servicePrev);
    if (LIKELY(service != servicePrev))
    {
      optionalServices.push_back(service);
    }
    servicePrev = service;
  }
  LOG4CXX_INFO(_logger,
               "GETOPTIONALSERVICES: NumRows " << res.numRows() << " Time = " << stopTimer() << " ("
                                               << stopCPU() << ") mSecs");
  res.freeResult();
}

///////////////////////////////////////////////////////////
//  QueryGetOptionalServicesHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetOptionalServicesHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetOptionalServicesHistorical"));
std::string QueryGetOptionalServicesHistorical::_baseSQL;
bool QueryGetOptionalServicesHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetOptionalServicesHistorical> g_GetOptionalServicesHistorical;

const char*
QueryGetOptionalServicesHistorical::getQueryName() const
{
  return "GETOPTIONALSERVICESHISTORICAL";
}

void
QueryGetOptionalServicesHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetOptionalServicesHistoricalSQLStatement<QueryGetOptionalServicesHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETOPTIONALSERVICESHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetOptionalServicesHistorical::findOptionalServicesInfo(
    std::vector<OptionalServicesInfo*>& optionalServices,
    const VendorCode& vendor,
    const CarrierCode& carrier,
    const Indicator& fltTktMerchInd,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(vendor, 1);
  substParm(carrier, 2);
  substParm(3, fltTktMerchInd);
  substParm(4, startDate);
  substParm(5, endDate);

  substParm(vendor, 5);
  substParm(carrier, 6);
  substParm(7, fltTktMerchInd);
  substParm(8, startDate);
  substParm(9, endDate);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  OptionalServicesInfo* service = nullptr;
  OptionalServicesInfo* servicePrev = nullptr;
  while ((row = res.nextRow()))
  {
    service = QueryGetOptionalServicesHistoricalSQLStatement<
        QueryGetOptionalServicesHistorical>::mapRowToOptionalServicesInfo(row, servicePrev);
    if (service != servicePrev)
    {
      optionalServices.push_back(service);
    }
    servicePrev = service;
  }
  LOG4CXX_INFO(_logger,
               "GETOPTIONALSERVICESHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
}

} // tse
