//----------------------------------------------------------------------------
//  File:           QueryGetTariffMileageAddon.cpp
//  Description:    QueryGetTariffMileageAddon
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

#include "DBAccess/Queries/QueryGetTariffMileageAddon.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetTariffMileageAddonSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetTariffMileageAddon::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTariffMileageAddon"));
std::string QueryGetTariffMileageAddon::_baseSQL;
bool QueryGetTariffMileageAddon::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTariffMileageAddon> g_GetTariffMileageAddon;

const char*
QueryGetTariffMileageAddon::getQueryName() const
{
  return "GETTARIFFMILEAGEADDON";
}

void
QueryGetTariffMileageAddon::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTariffMileageAddonSQLStatement<QueryGetTariffMileageAddon> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTARIFFMILEAGEADDON");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTariffMileageAddon::findTariffMileageAddon(std::vector<tse::TariffMileageAddon*>& lstTMA,
                                                   const CarrierCode& carrier,
                                                   const LocCode& unpublishedAddonLoc,
                                                   const GlobalDirection& globalDir)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  std::string gd;
  globalDirectionToStr(gd, globalDir);

  substParm(1, carrier);
  substParm(2, unpublishedAddonLoc);
  substParm(3, gd);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstTMA.push_back(QueryGetTariffMileageAddonSQLStatement<
        QueryGetTariffMileageAddon>::mapRowToTariffMileageAddon(row));
  }
  LOG4CXX_INFO(_logger,
               "GETTARIFFMILEAGEADDON: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                   << " (" << stopCPU() << ")");
  res.freeResult();
} // findTariffMileageAddon()

///////////////////////////////////////////////////////////
//
//  QueryGetTariffMileageAddonHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetTariffMileageAddonHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTariffMileageAddonHistorical"));
std::string QueryGetTariffMileageAddonHistorical::_baseSQL;
bool QueryGetTariffMileageAddonHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTariffMileageAddonHistorical> g_GetTariffMileageAddonHistorical;

const char*
QueryGetTariffMileageAddonHistorical::getQueryName() const
{
  return "GETTARIFFMILEAGEADDONHISTORICAL";
}

void
QueryGetTariffMileageAddonHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTariffMileageAddonHistoricalSQLStatement<QueryGetTariffMileageAddonHistorical>
    sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTARIFFMILEAGEADDONHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTariffMileageAddonHistorical::findTariffMileageAddon(
    std::vector<tse::TariffMileageAddon*>& lstTMA,
    const CarrierCode& carrier,
    const LocCode& unpublishedAddonLoc,
    const GlobalDirection& globalDir)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  std::string gd;
  globalDirectionToStr(gd, globalDir);

  substParm(1, carrier);
  substParm(2, unpublishedAddonLoc);
  substParm(3, gd);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstTMA.push_back(QueryGetTariffMileageAddonHistoricalSQLStatement<
        QueryGetTariffMileageAddonHistorical>::mapRowToTariffMileageAddon(row));
  }
  LOG4CXX_INFO(_logger,
               "GETTARIFFMILEAGEADDONHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findTariffMileageAddon()

///////////////////////////////////////////////////////////
//
//  QueryGetAllTariffMileageAddon
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllTariffMileageAddon::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllTariffMileageAddon"));
std::string QueryGetAllTariffMileageAddon::_baseSQL;
bool QueryGetAllTariffMileageAddon::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllTariffMileageAddon> g_GetAllTariffMileageAddon;

const char*
QueryGetAllTariffMileageAddon::getQueryName() const
{
  return "GETALLTARIFFMILEAGEADDON";
}

void
QueryGetAllTariffMileageAddon::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllTariffMileageAddonSQLStatement<QueryGetAllTariffMileageAddon> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLTARIFFMILEAGEADDON");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllTariffMileageAddon::findAllTariffMileageAddon(
    std::vector<tse::TariffMileageAddon*>& lstTMA)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstTMA.push_back(QueryGetAllTariffMileageAddonSQLStatement<
        QueryGetAllTariffMileageAddon>::mapRowToTariffMileageAddon(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLTARIFFMILEAGEADDON: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllTariffMileageAddon()
///////////////////////////////////////////////////////////
//
//  QueryGetAllTariffMileageAddonHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllTariffMileageAddonHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllTariffMileageAddonHistorical"));
std::string QueryGetAllTariffMileageAddonHistorical::_baseSQL;
bool QueryGetAllTariffMileageAddonHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllTariffMileageAddonHistorical>
g_GetAllTariffMileageAddonHistorical;

const char*
QueryGetAllTariffMileageAddonHistorical::getQueryName() const
{
  return "GETALLTARIFFMILEAGEADDONHISTORICAL";
}

void
QueryGetAllTariffMileageAddonHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllTariffMileageAddonHistoricalSQLStatement<QueryGetAllTariffMileageAddonHistorical>
    sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLTARIFFMILEAGEADDONHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllTariffMileageAddonHistorical::findAllTariffMileageAddon(
    std::vector<tse::TariffMileageAddon*>& lstTMA)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstTMA.push_back(QueryGetAllTariffMileageAddonHistoricalSQLStatement<
        QueryGetAllTariffMileageAddonHistorical>::mapRowToTariffMileageAddon(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLTARIFFMILEAGEADDONHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllTariffMileageAddon()
}
