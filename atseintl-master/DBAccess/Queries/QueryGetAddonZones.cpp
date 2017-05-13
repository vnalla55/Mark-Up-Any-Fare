//----------------------------------------------------------------------------
//  File:           QueryGetAddonZones.cpp
//  Description:    QueryGetAddonZones
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

#include "DBAccess/Queries/QueryGetAddonZones.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetAddonZonesSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetAddonZones::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAddonZones"));
std::string QueryGetAddonZones::_baseSQL;
bool QueryGetAddonZones::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAddonZones> g_GetAddonZones;

const char*
QueryGetAddonZones::getQueryName() const
{
  return "GETADDONZONES";
}

void
QueryGetAddonZones::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAddonZonesSQLStatement<QueryGetAddonZones> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETADDONZONES");

    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAddonZones::findAddonZoneInfo(std::vector<tse::AddonZoneInfo*>& addZones,
                                      const VendorCode& vendor,
                                      const CarrierCode& carrier,
                                      const LocCode& market)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(vendor, 1);
  substParm(carrier, 2);
  substParm(market, 3);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    addZones.push_back(
        QueryGetAddonZonesSQLStatement<QueryGetAddonZones>::mapRowToAddonZoneInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETADDONZONES: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                           << stopCPU() << ")");
  res.freeResult();
} // findAddonZoneInfo(#1)

///////////////////////////////////////////////////////////
//
//  QueryGetAddonZoneInfo
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAddonZoneInfo::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAddonZoneInfo"));
std::string QueryGetAddonZoneInfo::_baseSQL;
bool QueryGetAddonZoneInfo::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAddonZoneInfo> g_GetAddonZoneInfo;

const char*
QueryGetAddonZoneInfo::getQueryName() const
{
  return "GETADDONZONEINFO";
}

void
QueryGetAddonZoneInfo::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAddonZoneInfoSQLStatement<QueryGetAddonZoneInfo> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETADDONZONEINFO");

    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAddonZoneInfo::findAddonZoneInfo(std::vector<tse::AddonZoneInfo*>& addZones,
                                         const VendorCode& vendor,
                                         const CarrierCode& carrier,
                                         const TariffNumber& fareTariff,
                                         const AddonZone& zone)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, vendor);
  substParm(2, carrier);
  substParm(3, fareTariff);
  substParm(4, zone);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    addZones.push_back(
        QueryGetAddonZoneInfoSQLStatement<QueryGetAddonZoneInfo>::mapRowToAddonZoneInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETADDONZONEINFO: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                              << stopCPU() << ")");
  res.freeResult();
} // findAddonZoneInfo(#2)

///////////////////////////////////////////////////////////
//
//  QueryGetAllAddonZones
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllAddonZones::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllAddonZones"));
std::string QueryGetAllAddonZones::_baseSQL;
bool QueryGetAllAddonZones::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllAddonZones> g_GetAllAddonZones;

const char*
QueryGetAllAddonZones::getQueryName() const
{
  return "GETALLADDONZONES";
}

void
QueryGetAllAddonZones::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllAddonZonesSQLStatement<QueryGetAllAddonZones> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLADDONZONES");

    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllAddonZones::findAllAddonZoneInfo(std::vector<tse::AddonZoneInfo*>& addZones)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    addZones.push_back(
        QueryGetAllAddonZonesSQLStatement<QueryGetAllAddonZones>::mapRowToAddonZoneInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLADDONZONES: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                              << stopCPU() << ")");
  res.freeResult();
} // findAllAddonZoneInfo()

///////////////////////////////////////////////////////////
//
//  QueryGetAddonZonesHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAddonZonesHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAddonZonesHistorical"));
std::string QueryGetAddonZonesHistorical::_baseSQL;
bool QueryGetAddonZonesHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAddonZonesHistorical> g_GetAddonZonesHistorical;

const char*
QueryGetAddonZonesHistorical::getQueryName() const
{
  return "GETADDONZONESHISTORICAL";
}

void
QueryGetAddonZonesHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAddonZonesHistoricalSQLStatement<QueryGetAddonZonesHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETADDONZONESHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAddonZonesHistorical::findAddonZoneInfo(std::vector<tse::AddonZoneInfo*>& addZones,
                                                const VendorCode& vendor,
                                                const CarrierCode& carrier,
                                                const LocCode& market,
                                                const DateTime& startDate,
                                                const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(vendor, 1);
  substParm(carrier, 2);
  substParm(market, 3);
  substParm(4, startDate);
  substParm(5, endDate);
  substParm(6, endDate);
  substParm(vendor, 7);
  substParm(carrier, 8);
  substParm(market, 9);
  substParm(10, startDate);
  substParm(11, endDate);
  substParm(12, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    addZones.push_back(QueryGetAddonZonesHistoricalSQLStatement<
        QueryGetAddonZonesHistorical>::mapRowToAddonZoneInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETADDONZONESHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                     << " (" << stopCPU() << ")");
  res.freeResult();
} // findAddonZoneInfo(#1)

///////////////////////////////////////////////////////////
//
//  QueryGetAddonZoneInfoHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAddonZoneInfoHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAddonZoneInfoHistorical"));
std::string QueryGetAddonZoneInfoHistorical::_baseSQL;
bool QueryGetAddonZoneInfoHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAddonZoneInfoHistorical> g_GetAddonZoneInfoHistorical;

const char*
QueryGetAddonZoneInfoHistorical::getQueryName() const
{
  return "GETADDONZONEINFOHISTORICAL";
}

void
QueryGetAddonZoneInfoHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAddonZoneInfoHistoricalSQLStatement<QueryGetAddonZoneInfoHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETADDONZONEINFOHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAddonZoneInfoHistorical::findAddonZoneInfo(std::vector<tse::AddonZoneInfo*>& addZones,
                                                   const VendorCode& vendor,
                                                   const CarrierCode& carrier,
                                                   const TariffNumber& fareTariff,
                                                   const AddonZone& zone,
                                                   const DateTime& startDate,
                                                   const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, vendor);
  substParm(2, carrier);
  substParm(3, fareTariff);
  substParm(4, zone);
  substParm(5, startDate);
  substParm(6, endDate);
  substParm(7, endDate);
  substParm(8, vendor);
  substParm(9, carrier);
  substParm(10, fareTariff);
  substParm(11, zone);
  substParm(12, startDate);
  substParm(13, endDate);
  substParm(14, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    addZones.push_back(QueryGetAddonZoneInfoHistoricalSQLStatement<
        QueryGetAddonZoneInfoHistorical>::mapRowToAddonZoneInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETADDONZONEINFOHISTORICAL: NumRows = " << res.numRows() << " Time = "
                                                        << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findAddonZoneInfo(#2)
}
