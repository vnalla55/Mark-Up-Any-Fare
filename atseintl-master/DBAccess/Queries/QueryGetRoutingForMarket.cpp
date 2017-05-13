//----------------------------------------------------------------------------
//  File:           QueryGetRoutingForMarket.cpp
//  Description:    QueryGetRoutingForMarket
//  Created:        5/24/2006
//  Authors:         Mike Lillis
//
//  Updates:
//
//  2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//  and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//  or transfer of this software/documentation, in any medium, or incorporation of this
//  software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetRoutingForMarket.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetRoutingForMarketSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetRoutingForDomMarket::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetRoutingForDomMarket"));
std::string QueryGetRoutingForDomMarket::_baseSQL;
bool QueryGetRoutingForDomMarket::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetRoutingForDomMarket> g_GetRoutingForDomMarket;
std::vector<LocCode> QueryGetRoutingForDomMarket::domMkts;
boost::mutex QueryGetRoutingForDomMarket::_mutex;

log4cxx::LoggerPtr
QueryGetDomMarkets_Rtg::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetRoutingForDomMarket.GetDomMarkets_Rtg"));
std::string QueryGetDomMarkets_Rtg::_baseSQL;
bool QueryGetDomMarkets_Rtg::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetDomMarkets_Rtg> g_GetDomMarkets_Rtg;

log4cxx::LoggerPtr
QueryGetRoutingForIntlMarket::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetRoutingForDomMarket.GetRoutingForIntlMarket"));
std::string QueryGetRoutingForIntlMarket::_baseSQL;
bool QueryGetRoutingForIntlMarket::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetRoutingForIntlMarket> g_GetRoutingForIntlMarket;

const char*
QueryGetRoutingForDomMarket::getQueryName() const
{
  return "GETROUTINGFORDOMMARKET";
}

void
QueryGetRoutingForDomMarket::initialize()
{
  if (!_isInitialized)
  {
    QueryGetRoutingForDomMarketSQLStatement<QueryGetRoutingForDomMarket> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETROUTINGFORDOMMARKET");
    substTableDef(&_baseSQL);

    QueryGetDomMarkets_Rtg::initialize();
    QueryGetRoutingForIntlMarket::initialize();
    _isInitialized = true;
  }
} // initialize()

void
QueryGetRoutingForDomMarket::findRoutingForMarket(std::vector<tse::RoutingKeyInfo*>& infos,
                                                  LocCode& market1,
                                                  LocCode& market2,
                                                  CarrierCode& carrier)
{
  boost::lock_guard<boost::mutex> g(_mutex);
  if (domMkts.size() == 0)
  { // Must be 1st time thru, so get static list of Domestic Mkts
    QueryGetDomMarkets_Rtg SQLDomMkts(_dbAdapt);
    SQLDomMkts.getDomMarkets(domMkts);
    resetSQL(); // Reset here for getDomRoutings()
  }

  if (market1 != market2 && std::binary_search(domMkts.begin(), domMkts.end(), market1) &&
      std::binary_search(domMkts.begin(), domMkts.end(), market2))
  {
    getDomRoutings(infos, market1, market2, carrier);
  }
  else
  {
    QueryGetRoutingForIntlMarket SQLIntlRoutings(_dbAdapt);
    SQLIntlRoutings.getIntlRoutings(infos, market1, market2, carrier);
  }
} // QueryGetRoutingForDomMarket::findRoutingForMarket()

void
QueryGetRoutingForDomMarket::getDomRoutings(std::vector<tse::RoutingKeyInfo*>& infos,
                                            LocCode& market1,
                                            LocCode& market2,
                                            CarrierCode& carrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(market1, 1);
  substParm(market2, 2);
  substParm(carrier, 3);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    infos.push_back(QueryGetRoutingForDomMarketSQLStatement<
        QueryGetRoutingForDomMarket>::mapRowToRoutingKeyInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETROUTINGFORDOMMARKET: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                    << " (" << stopCPU() << ") mSecs");
  res.freeResult();
} // QueryGetRoutingForDomMarket::getDomRoutings()

///////////////////////////////////////////////////////////
//  QueryGetRoutingForDomMarketHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetRoutingForDomMarketHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetRoutingForDomMarketHistorical"));
std::string QueryGetRoutingForDomMarketHistorical::_baseSQL;
bool QueryGetRoutingForDomMarketHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetRoutingForDomMarketHistorical> g_GetRoutingForDomMarketHistorical;

log4cxx::LoggerPtr
QueryGetRoutingForIntlMarketHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetRoutingForDomMarketHist.GetRoutingForIntlMarketHistorical"));
std::string QueryGetRoutingForIntlMarketHistorical::_baseSQL;
bool QueryGetRoutingForIntlMarketHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetRoutingForIntlMarketHistorical>
g_GetRoutingForIntlMarketHistorical;

const char*
QueryGetRoutingForDomMarketHistorical::getQueryName() const
{
  return "GETROUTINGFORDOMMARKETHISTORICAL";
}

void
QueryGetRoutingForDomMarketHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetRoutingForDomMarketHistoricalSQLStatement<QueryGetRoutingForDomMarketHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETROUTINGFORDOMMARKETHISTORICAL");
    substTableDef(&_baseSQL);

    QueryGetRoutingForIntlMarketHistorical::initialize();
    _isInitialized = true;
  }
} // initialize()

void
QueryGetRoutingForDomMarketHistorical::findRoutingForMarket(
    std::vector<tse::RoutingKeyInfo*>& infos,
    LocCode& market1,
    LocCode& market2,
    CarrierCode& carrier,
    const DateTime& startDate,
    const DateTime& endDate)
{
  boost::lock_guard<boost::mutex> g(_mutex);
  if (domMkts.size() == 0)
  { // Must be 1st time thru, so get static list of Domestic Mkts
    QueryGetDomMarkets_Rtg SQLDomMkts(_dbAdapt);
    SQLDomMkts.getDomMarkets(domMkts);
    resetSQL(); // Reset here for getDomRoutings()
  }

  if (market1 != market2 && std::binary_search(domMkts.begin(), domMkts.end(), market1) &&
      std::binary_search(domMkts.begin(), domMkts.end(), market2))
  {
    getDomRoutings(infos, market1, market2, carrier, startDate, endDate);
  }
  else
  {
    QueryGetRoutingForIntlMarketHistorical SQLIntlRoutings(_dbAdapt);
    SQLIntlRoutings.getIntlRoutings(infos, market1, market2, carrier, startDate, endDate);
  }
} // QueryGetRoutingForDomMarketHistorical::findRoutingForMarket()

void
QueryGetRoutingForDomMarketHistorical::getDomRoutings(std::vector<tse::RoutingKeyInfo*>& infos,
                                                      LocCode& market1,
                                                      LocCode& market2,
                                                      CarrierCode& carrier,
                                                      const DateTime& startDate,
                                                      const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(market1, 1);
  substParm(market2, 2);
  substParm(carrier, 3);
  substParm(4, startDate);
  substParm(5, endDate);
  substParm(6, startDate);
  substParm(7, endDate);
  substParm(8, endDate);
  substParm(9, startDate);
  substParm(10, endDate);
  substParm(11, endDate);
  substParm(market1, 12);
  substParm(market2, 13);
  substParm(carrier, 14);
  substParm(15, startDate);
  substParm(16, endDate);
  substParm(17, startDate);
  substParm(18, endDate);
  substParm(19, endDate);
  substParm(20, startDate);
  substParm(21, endDate);
  substParm(22, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    infos.push_back(QueryGetRoutingForDomMarketHistoricalSQLStatement<
        QueryGetRoutingForDomMarketHistorical>::mapRowToRoutingKeyInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETROUTINGFORDOMMARKETHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ") mSecs");
  res.freeResult();
} // QueryGetRoutingForDomMarketHistorical::getDomRoutings()

///////////////////////////////////////////////////////////
//  QueryGetDomMarkets_Rtg
///////////////////////////////////////////////////////////
const char*
QueryGetDomMarkets_Rtg::getQueryName() const
{
  return "GETROUTINGMARKETS";
}

void
QueryGetDomMarkets_Rtg::initialize()
{
  if (!_isInitialized)
  {
    QueryGetDomMarkets_RtgSQLStatement<QueryGetDomMarkets_Rtg> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETROUTINGMARKETS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetDomMarkets_Rtg::getDomMarkets(std::vector<LocCode> domMkts)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    domMkts.push_back(
        QueryGetDomMarkets_RtgSQLStatement<QueryGetDomMarkets_Rtg>::mapRowToMarket(row));
  }
  LOG4CXX_INFO(_logger,
               "GETDOMMARKETS: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                           << stopCPU() << ")");
  res.freeResult();
} // QueryGetDomMarkets_Rtg::getDomMarkets()

///////////////////////////////////////////////////////////
//  QueryGetRoutingForIntlMarket
///////////////////////////////////////////////////////////
const char*
QueryGetRoutingForIntlMarket::getQueryName() const
{
  return "GETROUTINGFORINTLMARKET";
}

void
QueryGetRoutingForIntlMarket::initialize()
{
  if (!_isInitialized)
  {
    QueryGetRoutingForIntlMarketSQLStatement<QueryGetRoutingForIntlMarket> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETROUTINGFORINTLMARKET");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetRoutingForIntlMarket::getIntlRoutings(std::vector<tse::RoutingKeyInfo*>& infos,
                                              LocCode& market1,
                                              LocCode& market2,
                                              CarrierCode& carrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(market1, 1);
  substParm(market2, 2);
  substParm(carrier, 3);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    infos.push_back(QueryGetRoutingForIntlMarketSQLStatement<
        QueryGetRoutingForIntlMarket>::mapRowToRoutingKeyInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETROUTINGFORINTLMARKET: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                     << " (" << stopCPU() << ") mSecs");
  res.freeResult();
} // QueryGetRoutingForIntlMarket::getIntlRoutings()

///////////////////////////////////////////////////////////
//  QueryGetRoutingForIntlMarketHistorical
///////////////////////////////////////////////////////////
const char*
QueryGetRoutingForIntlMarketHistorical::getQueryName() const
{
  return "GETROUTINGFORINTLMARKETHISTORICAL";
}

void
QueryGetRoutingForIntlMarketHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetRoutingForIntlMarketHistoricalSQLStatement<QueryGetRoutingForIntlMarketHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETROUTINGFORINTLMARKETHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetRoutingForIntlMarketHistorical::getIntlRoutings(std::vector<tse::RoutingKeyInfo*>& infos,
                                                        LocCode& market1,
                                                        LocCode& market2,
                                                        CarrierCode& carrier,
                                                        const DateTime& startDate,
                                                        const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(market1, 1);
  substParm(market2, 2);
  substParm(carrier, 3);
  substParm(4, startDate);
  substParm(5, endDate);
  substParm(6, startDate);
  substParm(7, endDate);
  substParm(8, endDate);
  substParm(9, startDate);
  substParm(10, endDate);
  substParm(11, endDate);
  substParm(market1, 12);
  substParm(market2, 13);
  substParm(carrier, 14);
  substParm(15, startDate);
  substParm(16, endDate);
  substParm(17, startDate);
  substParm(18, endDate);
  substParm(19, endDate);
  substParm(20, startDate);
  substParm(21, endDate);
  substParm(22, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    infos.push_back(QueryGetRoutingForIntlMarketHistoricalSQLStatement<
        QueryGetRoutingForIntlMarketHistorical>::mapRowToRoutingKeyInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETROUTINGFORINTLMARKETHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ") mSecs");
  res.freeResult();
} // QueryGetRoutingForIntlMarketHistorical::getIntlRoutings()
}
