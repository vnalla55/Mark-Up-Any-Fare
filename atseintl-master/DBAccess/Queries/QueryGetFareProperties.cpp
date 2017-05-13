//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetFareProperties.h"

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFarePropertiesSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetFareProperties::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetFareProperties"));
std::string QueryGetFareProperties::_baseSQL;
bool QueryGetFareProperties::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFareProperties> g_GetFareProperties;

const char*
QueryGetFareProperties::getQueryName() const
{
  return "GETFAREPROPERTIES";
}

void
QueryGetFareProperties::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFarePropertiesSQLStatement<QueryGetFareProperties> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFAREPROPERTIES");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetFareProperties::findFareProperties(std::vector<FareProperties*>& fareProperties,
                                           const VendorCode& vendor,
                                           const CarrierCode& carrier,
                                           const TariffNumber& tariff,
                                           const RuleNumber& rule)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(vendor, 1);
  substParm(carrier, 2);
  substParm(3, tariff);
  substParm(rule, 4);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    fareProperties.push_back(
        QueryGetFarePropertiesSQLStatement<QueryGetFareProperties>::mapRowToFareProperties(row));
  }
  LOG4CXX_INFO(_logger,
               "GETFAREPROPERTIES: NumRows " << res.numRows() << " Time = " << stopTimer() << " ("
                                             << stopCPU() << ") mSecs");
  res.freeResult();
}

///////////////////////////////////////////////////////////
//  QueryGetFarePropertiesHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetFarePropertiesHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetFarePropertiesHistorical"));
std::string QueryGetFarePropertiesHistorical::_baseSQL;
bool QueryGetFarePropertiesHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFarePropertiesHistorical> g_GetFarePropertiesHistorical;

const char*
QueryGetFarePropertiesHistorical::getQueryName() const
{
  return "GETFAREPROPERTIESHISTORICAL";
}

void
QueryGetFarePropertiesHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFarePropertiesHistoricalSQLStatement<QueryGetFarePropertiesHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFAREPROPERTIESHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetFarePropertiesHistorical::findFareProperties(std::vector<FareProperties*>& fareProperties,
                                                     const VendorCode& vendor,
                                                     const CarrierCode& carrier,
                                                     const TariffNumber& tariff,
                                                     const RuleNumber& rule,
                                                     const DateTime& startDate,
                                                     const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(vendor, 1);
  substParm(carrier, 2);
  substParm(3, tariff);
  substParm(rule, 4);
  substParm(5, startDate);
  substParm(6, endDate);
  substParm(vendor, 7);
  substParm(carrier, 8);
  substParm(9, tariff);
  substParm(rule, 10);
  substParm(11, startDate);
  substParm(12, endDate);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    fareProperties.push_back(QueryGetFarePropertiesHistoricalSQLStatement<
        QueryGetFarePropertiesHistorical>::mapRowToFareProperties(row));
  }
  LOG4CXX_INFO(_logger,
               "GETFAREPROPERTIESHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
}
} // tse
