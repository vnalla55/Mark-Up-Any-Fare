//-------------------------------------------------------------------------------
// Copyright 2011, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetOptionalServicesMkt.h"

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetOptionalServicesSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetOptionalServicesMkt::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetOptionalServicesMkt"));
std::string QueryGetOptionalServicesMkt::_baseSQL;
bool QueryGetOptionalServicesMkt::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetOptionalServicesMkt> g_GetOptionalServicesMkt;

const char*
QueryGetOptionalServicesMkt::getQueryName() const
{
  return "GETOPTIONALSERVICESMKT";
}

void
QueryGetOptionalServicesMkt::initialize()
{
  if (!_isInitialized)
  {
    QueryGetOptionalServicesMktSQLStatement<QueryGetOptionalServicesMkt> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETOPTIONALSERVICESMKT");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetOptionalServicesMkt::findOptionalServicesMktInfo(
    std::vector<OptionalServicesInfo*>& optionalServices,
    const VendorCode& vendor,
    const CarrierCode& carrier,
    const LocCode& loc1,
    const LocCode& loc2)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(vendor, 1);
  substParm(carrier, 2);
  if (loc1.empty() && loc2.empty())
  {
    substParm(" ", 3);
    substParm(" ", 4);
    substParm(" ", 5);
    substParm(" ", 6);
  }
  else
  {
    substParm("P", 3);
    substParm(loc1, 4);
    substParm("P", 5);
    substParm(loc2, 6);
  }
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  OptionalServicesInfo* service = nullptr;
  OptionalServicesInfo* servicePrev = nullptr;
  while ((row = res.nextRow()))
  {
    service = QueryGetOptionalServicesMktSQLStatement<
        QueryGetOptionalServicesMkt>::mapRowToOptionalServicesInfo(row, servicePrev);
    if (service != servicePrev)
    {
      optionalServices.push_back(service);
    }
    servicePrev = service;
  }
  LOG4CXX_INFO(_logger,
               "GETOPTIONALSERVICESMKT: NumRows " << res.numRows() << " Time = " << stopTimer()
                                                  << " (" << stopCPU() << ") mSecs");
  res.freeResult();
}

///////////////////////////////////////////////////////////
//  QueryGetOptionalServicesMktHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetOptionalServicesMktHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetOptionalServicesMktHistorical"));
std::string QueryGetOptionalServicesMktHistorical::_baseSQL;
bool QueryGetOptionalServicesMktHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetOptionalServicesMktHistorical> g_GetOptionalServicesMktHistorical;

const char*
QueryGetOptionalServicesMktHistorical::getQueryName() const
{
  return "GETOPTIONALSERVICESMKTHISTORICAL";
}

void
QueryGetOptionalServicesMktHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetOptionalServicesMktHistoricalSQLStatement<QueryGetOptionalServicesMktHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETOPTIONALSERVICESMKTHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetOptionalServicesMktHistorical::findOptionalServicesMktInfo(
    std::vector<OptionalServicesInfo*>& optionalServices,
    const VendorCode& vendor,
    const CarrierCode& carrier,
    const LocCode& loc1,
    const LocCode& loc2,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(vendor, 1);
  substParm(carrier, 2);
  if (loc1.empty() && loc2.empty())
  {
    substParm(" ", 3);
    substParm(" ", 4);
    substParm(" ", 5);
    substParm(" ", 6);
  }
  else
  {
    substParm("P", 3);
    substParm(loc1, 4);
    substParm("P", 5);
    substParm(loc2, 6);
  }
  substParm(7, startDate);
  substParm(8, endDate);

  substParm(vendor, 9);
  substParm(carrier, 10);
  if (loc1.empty() && loc2.empty())
  {
    substParm(" ", 11);
    substParm(" ", 12);
    substParm(" ", 13);
    substParm(" ", 14);
  }
  else
  {
    substParm("P", 11);
    substParm(loc1, 12);
    substParm("P", 13);
    substParm(loc2, 14);
  }
  substParm(15, startDate);
  substParm(16, endDate);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  OptionalServicesInfo* service = nullptr;
  OptionalServicesInfo* servicePrev = nullptr;
  while ((row = res.nextRow()))
  {
    service = QueryGetOptionalServicesMktHistoricalSQLStatement<
        QueryGetOptionalServicesMktHistorical>::mapRowToOptionalServicesInfo(row, servicePrev);
    if (service != servicePrev)
    {
      optionalServices.push_back(service);
    }
    servicePrev = service;
  }
  LOG4CXX_INFO(_logger,
               "GETOPTIONALSERVICESMKTHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
}
} // tse
