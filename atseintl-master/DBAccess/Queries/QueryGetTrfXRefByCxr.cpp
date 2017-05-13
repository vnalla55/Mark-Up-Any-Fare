//----------------------------------------------------------------------------
//  File:           QueryGetTrfXRefByCxr.cpp
//  Description:    QueryGetTrfXRefByCxr
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
#include "DBAccess/Queries/QueryGetTrfXRefByCxr.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetTrfXRefByCxrSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetTrfXRefByCxr::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTrfXRefByCxr"));
std::string QueryGetTrfXRefByCxr::_baseSQL;
bool QueryGetTrfXRefByCxr::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTrfXRefByCxr> g_GetTrfXRefByCxr;

const char*
QueryGetTrfXRefByCxr::getQueryName() const
{
  return "GETTRFXREFBYCXR";
}

void
QueryGetTrfXRefByCxr::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTrfXRefByCxrSQLStatement<QueryGetTrfXRefByCxr> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTRFXREFBYCXR");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTrfXRefByCxr::findTariffXRefByCarrier(std::vector<tse::TariffCrossRefInfo*>& tarList,
                                              CarrierCode& carrier,
                                              VendorCode& vendor,
                                              std::string& tariffCrossRefType)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(carrier, 1);
  substParm(vendor, 2);
  substParm(tariffCrossRefType, 3);
#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
  substCurrentDate(false/*use whole timestamp, not only date part*/);
#else
  substCurrentDate();
#endif
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    tarList.push_back(
        QueryGetTrfXRefByCxrSQLStatement<QueryGetTrfXRefByCxr>::mapRowToTariffCrossRefInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETTRFXREFBYCXR: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                             << stopCPU() << ")");
  res.freeResult();
} // findTariffXRefByCarrier()

///////////////////////////////////////////////////////////
//  QueryGetTrfXRefByCxrHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetTrfXRefByCxrHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetTrfXRefByCxrHistorical"));
std::string QueryGetTrfXRefByCxrHistorical::_baseSQL;
bool QueryGetTrfXRefByCxrHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTrfXRefByCxrHistorical> g_GetTrfXRefByCxrHistorical;

const char*
QueryGetTrfXRefByCxrHistorical::getQueryName() const
{
  return "GETTRFXREFBYCXRHISTORICAL";
}

void
QueryGetTrfXRefByCxrHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTrfXRefByCxrHistoricalSQLStatement<QueryGetTrfXRefByCxrHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTRFXREFBYCXRHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTrfXRefByCxrHistorical::findTariffXRefByCarrier(
    std::vector<tse::TariffCrossRefInfo*>& tarList,
    CarrierCode& carrier,
    VendorCode& vendor,
    std::string& tariffCrossRefType,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(carrier, 1);
  substParm(vendor, 2);
  substParm(tariffCrossRefType, 3);
  substParm(4, startDate);
  substParm(5, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    tarList.push_back(QueryGetTrfXRefByCxrHistoricalSQLStatement<
        QueryGetTrfXRefByCxrHistorical>::mapRowToTariffCrossRefInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETTRFXREFBYCXRHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                       << " (" << stopCPU() << ")");
  res.freeResult();
} // findTariffXRefByCarrier()
}
