//----------------------------------------------------------------------------
//
//  File:           QueryGetAddonMarketCarriers.cpp
//  Description:    QueryGetAddonMarketCarriers
//  Created:        5/24/2006
//  Authors:         Mike Lillis
//
//  Updates:
//
//  2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//  and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//  or transfer of this software/documentation, in any medium, or incorporation of this
//  software/documentation into any system or publication, is strictly prohibited
//
// ----------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetAddonMarketCarriers.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetAddonMarketCarriersSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetAddonMarketCarriers::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAddonMarketCarriers"));
std::string QueryGetAddonMarketCarriers::_baseSQL;
bool QueryGetAddonMarketCarriers::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAddonMarketCarriers> g_GetAddonMarketCarriers;

const char*
QueryGetAddonMarketCarriers::getQueryName() const
{
  return "GETADDONMARKETCARRIERS";
}

void
QueryGetAddonMarketCarriers::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAddonMarketCarriersSQLStatement<QueryGetAddonMarketCarriers> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETADDONMARKETCARRIERS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAddonMarketCarriers::findAddOnMarketCarriers(std::vector<const tse::MarketCarrier*>& lstMC,
                                                     const LocCode& market1,
                                                     const LocCode& market2)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(market1, 1);
  substParm(market2, 2);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    lstMC.push_back(QueryGetAddonMarketCarriersSQLStatement<
        QueryGetAddonMarketCarriers>::mapRowToAddOnMarketCarrier(row));
  }
  LOG4CXX_INFO(_logger,
               "GETADDONMARKETCARRIERS: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                    << " (" << stopCPU() << ") mSecs");
  res.freeResult();
} // findAddOnMarketCarriers()

///////////////////////////////////////////////////////////
//  QueryGetAddonMarketCarriersHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAddonMarketCarriersHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAddonMarketCarriersHistorical"));
std::string QueryGetAddonMarketCarriersHistorical::_baseSQL;
bool QueryGetAddonMarketCarriersHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAddonMarketCarriersHistorical> g_GetAddonMarketCarriersHistorical;

const char*
QueryGetAddonMarketCarriersHistorical::getQueryName() const
{
  return "GETADDONMARKETCARRIERSHISTORICAL";
}

void
QueryGetAddonMarketCarriersHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAddonMarketCarriersHistoricalSQLStatement<QueryGetAddonMarketCarriersHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETADDONMARKETCARRIERSHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAddonMarketCarriersHistorical::findAddOnMarketCarriers(
    std::vector<const tse::MarketCarrier*>& lstMC,
    const LocCode& market1,
    const LocCode& market2,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, market1);
  substParm(2, market2);
  substParm(3, startDate);
  substParm(4, endDate);
  substParm(5, endDate);
  substParm(6, market1);
  substParm(7, market2);
  substParm(8, startDate);
  substParm(9, endDate);
  substParm(10, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstMC.push_back(QueryGetAddonMarketCarriersHistoricalSQLStatement<
        QueryGetAddonMarketCarriersHistorical>::mapRowToAddOnMarketCarrier(row));
  }
  LOG4CXX_INFO(_logger,
               "GETADDONMARKETCARRIERSHISTORICAL: NumRows: "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ") mSecs");
  res.freeResult();
} // QueryGetAddonMarketCarriersHistorical::findAddOnMarketCarriers()
}
