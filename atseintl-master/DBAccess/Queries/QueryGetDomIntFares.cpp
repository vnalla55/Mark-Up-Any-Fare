//----------------------------------------------------------------------------
//              File:           QueryGetDomIntFares.cpp
//              Description:    QueryGetDomIntFares
//              Created:        5/24/2006
//     Authors:         Mike Lillis
//
//              Updates:
//
//         � 2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//         and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//         or transfer of this software/documentation, in any medium, or incorporation of this
//         software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetDomIntFares.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetDomIntFaresSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetDomFares::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetDomFares"));
std::string QueryGetDomFares::_baseSQL;
bool QueryGetDomFares::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetDomFares> g_GetDomFares;
std::vector<LocCode> QueryGetDomFares::_domMkts;
boost::mutex QueryGetDomFares::_mutex;

const char*
QueryGetDomFares::getQueryName() const
{
  return "GETDOMFARES";
}

void
QueryGetDomFares::initialize()
{
  if (!_isInitialized)
  {
    QueryGetDomFaresSQLStatement<QueryGetDomFares> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETDOMFARES");
    substTableDef(&_baseSQL);

    QueryGetDomMarkets_Fare::initialize();
    QueryGetIntFares::initialize();
    _isInitialized = true;
  }
} // initialize()

bool
QueryGetDomFares::isDomestic(const LocCode& market1, const LocCode& market2)
{
  DateTime today = DateTime::localTime();
  NationCode nation = getLocNationData(market1, today, today, false);
  if (nation != NATION_US && nation != NATION_CA)
    return false;
  nation = getLocNationData(market2, today, today, false);
  return (nation == NATION_US || nation == NATION_CA);
}

void
QueryGetDomFares::findFares(std::vector<const tse::FareInfo*>& lstFI,
                            const LocCode& market1,
                            const LocCode& market2,
                            const CarrierCode& carrier)
{
  if (market1 != market2 && isDomestic(market1, market2))
  {
    getDomFares(lstFI, market1, market2, carrier);
  }
  else
  {
    QueryGetIntFares SQLIntFares(_dbAdapt);
    SQLIntFares.getIntFares(lstFI, market1, market2, carrier);
  }
} // QueryGetDomFares::findFares(Single Carrier)

void
QueryGetDomFares::findFares(std::vector<const tse::FareInfo*>& lstFI,
                            const LocCode& market1,
                            const LocCode& market2,
                            const std::vector<CarrierCode>& carriers)
{
  if (market1 != market2 && isDomestic(market1, market2))
  {
    getDomFares(lstFI, market1, market2, carriers);
  }
  else
  {
    QueryGetIntFares SQLIntFares(_dbAdapt);
    SQLIntFares.getIntFares(lstFI, market1, market2, carriers);
  }
} // QueryGetDomFares::findFares(Multiple Carriers)

void
QueryGetDomFares::getDomFares(std::vector<const tse::FareInfo*>& lstFI,
                              const LocCode& market1,
                              const LocCode& market2,
                              const CarrierCode& carrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL(); // For after DomMkt Load AND between calls from mult cxrs.

  substParm(market1, 1);
  substParm(market2, 2);
  substParm(carrier, 3);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    lstFI.push_back(QueryGetDomFaresSQLStatement<QueryGetDomFares>::mapRowToFareInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETDOMFARES: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                         << stopCPU() << ") mSecs");
  res.freeResult();
} // QueryGetDomFares::getDomFares(Single Carrier)

void
QueryGetDomFares::getDomFares(std::vector<const tse::FareInfo*>& lstFI,
                              const LocCode& market1,
                              const LocCode& market2,
                              const std::vector<CarrierCode>& carriers)
{
  for (const auto carrier : carriers)
  {
    // create a new query each time through the loop, because
    // calling findDomFares makes the query object unreusable
    QueryGetDomFares SQLDomFares(_dbAdapt);
    SQLDomFares.getDomFares(lstFI, market1, market2, carrier);
  }

} // QueryGetDomFares::getDomFares(Multiple Carriers)

void
QueryGetDomFares::clearDomMkts()
{
  boost::lock_guard<boost::mutex> g(_mutex);
  _domMkts.clear();
} // QueryGetDomFares::clearDomMkts()

///////////////////////////////////////////////////////////
//
//  QueryGetDomFaresHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetDomFaresHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetDomFaresHistorical"));
std::string QueryGetDomFaresHistorical::_baseSQL;
bool QueryGetDomFaresHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetDomFaresHistorical> g_GetDomFaresHistorical;
std::vector<MarketAndDate> QueryGetDomFaresHistorical::_domMkts;
boost::mutex QueryGetDomFaresHistorical::_mutex;

const char*
QueryGetDomFaresHistorical::getQueryName() const
{
  return "GETDOMFARESHISTORICAL";
}

void
QueryGetDomFaresHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetDomFaresHistoricalSQLStatement<QueryGetDomFaresHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETDOMFARESHISTORICAL");
    substTableDef(&_baseSQL);

    QueryGetDomMarkets_FareHistorical::initialize();
    QueryGetIntFaresHistorical::initialize();
    _isInitialized = true;
  }
} // initialize()

QueryGetDomFaresHistorical::FareRegionType
QueryGetDomFaresHistorical::isDomestic(const LocCode& market1,
                                       const LocCode& market2,
                                       const DateTime& startDate,
                                       const DateTime& endDate)
{
  if (market1 == market2)
    return INTERNATIONAL_REGION;
  // We check the geography at the start and end dates for the date range
  // If a market changes twice (Domestic->International->Domestic) within that time, it would not be
  // detected
  NationCode nationAtStart = getLocNationData(market1, startDate, startDate, true);
  bool isMarket1DomesticAtStart = (nationAtStart == NATION_US || nationAtStart == NATION_CA);
  NationCode nationAtEnd = getLocNationData(market1, endDate, endDate, true);
  bool isMarket1DomesticAtEnd = (nationAtEnd == NATION_US || nationAtEnd == NATION_CA);
  if (isMarket1DomesticAtStart != isMarket1DomesticAtEnd)
  {
    LOG4CXX_INFO(_logger,
                 "IsDomestic: Market " << market1 << " is mixed, new, or deleted " << nationAtStart
                                       << " " << nationAtEnd);
    return MIXED_REGION;
  }
  nationAtStart = getLocNationData(market2, startDate, startDate, true);
  bool isMarket2DomesticAtStart = (nationAtStart == NATION_US || nationAtStart == NATION_CA);
  nationAtEnd = getLocNationData(market2, endDate, endDate, true);
  bool isMarket2DomesticAtEnd = (nationAtEnd == NATION_US || nationAtEnd == NATION_CA);
  if (isMarket2DomesticAtStart == isMarket2DomesticAtEnd)
    return (isMarket1DomesticAtStart && isMarket2DomesticAtStart) ? DOMESTIC_REGION
                                                                  : INTERNATIONAL_REGION;
  LOG4CXX_INFO(_logger,
               "IsDomestic: Market " << market1 << " is mixed, new, or deleted " << nationAtStart
                                     << " " << nationAtEnd);
  return MIXED_REGION;
}

void
QueryGetDomFaresHistorical::findFares(std::vector<const tse::FareInfo*>& lstFI,
                                      const LocCode& market1,
                                      const LocCode& market2,
                                      const CarrierCode& carrier,
                                      const DateTime& startDate,
                                      const DateTime& endDate)
{
  FareRegionType domesticFare = isDomestic(market1, market2, startDate, endDate);
  if (domesticFare == DOMESTIC_REGION)
  {
    getDomFares(lstFI, market1, market2, carrier, startDate, endDate);
  }
  else if (domesticFare == INTERNATIONAL_REGION)
  {
    QueryGetIntFaresHistorical SQLIntFares(_dbAdapt);
    SQLIntFares.getIntFares(lstFI, market1, market2, carrier, startDate, endDate);
  }
  else
  {
    QueryGetAllFaresHistorical SQLAllFares(_dbAdapt);
    SQLAllFares.getAllFares(lstFI, market1, market2, carrier, startDate, endDate);
  }
} // QueryGetDomFaresHistorical::findFares(Single Carrier)

void
QueryGetDomFaresHistorical::findFares(std::vector<const tse::FareInfo*>& lstFI,
                                      const LocCode& market1,
                                      const LocCode& market2,
                                      const std::vector<CarrierCode>& carriers,
                                      const DateTime& startDate,
                                      const DateTime& endDate)
{
  FareRegionType domesticFare = isDomestic(market1, market2, startDate, endDate);
  if (domesticFare == DOMESTIC_REGION)
  {
    getDomFares(lstFI, market1, market2, carriers, startDate, endDate);
  }
  else if (domesticFare == INTERNATIONAL_REGION)
  {
    QueryGetIntFaresHistorical SQLIntFares(_dbAdapt);
    SQLIntFares.getIntFares(lstFI, market1, market2, carriers, startDate, endDate);
  }
  else
  {
    QueryGetAllFaresHistorical SQLAllFares(_dbAdapt);
    SQLAllFares.getAllFares(lstFI, market1, market2, carriers, startDate, endDate);
  }
} // QueryGetDomFaresHistorical::findFares(Multiple Carriers)

void
QueryGetDomFaresHistorical::getDomFares(std::vector<const tse::FareInfo*>& lstFI,
                                        const LocCode& market1,
                                        const LocCode& market2,
                                        const CarrierCode& carrier,
                                        const DateTime& startDate,
                                        const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL(); // For after DomMkt Load AND between calls from mult cxrs.

  substParm(market1, 1);
  substParm(market2, 2);
  substParm(carrier, 3);
  substParm(4, startDate);
  substParm(5, endDate);
  substParm(market1, 6);
  substParm(market2, 7);
  substParm(carrier, 8);
  substParm(9, startDate);
  substParm(10, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    lstFI.push_back(
        QueryGetDomFaresHistoricalSQLStatement<QueryGetDomFaresHistorical>::mapRowToFareInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETDOMFARESHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                   << " (" << stopCPU() << ") mSecs");
  res.freeResult();
} // QueryGetDomFaresHistorical::getDomFares(Single Carrier)

void
QueryGetDomFaresHistorical::getDomFares(std::vector<const tse::FareInfo*>& lstFI,
                                        const LocCode& market1,
                                        const LocCode& market2,
                                        const std::vector<CarrierCode>& carriers,
                                        const DateTime& startDate,
                                        const DateTime& endDate)
{
  for (const auto carrier : carriers)
  {
    // create a new query each time through the loop, because
    // calling findDomFares makes the query object unreusable
    QueryGetDomFaresHistorical SQLDomFares(_dbAdapt);
    SQLDomFares.getDomFares(lstFI, market1, market2, carrier, startDate, endDate);
  }
} // QueryGetDomFaresHistorical::getDomFares(Multiple Carriers)

void
QueryGetDomFaresHistorical::clearDomMkts()
{
  boost::lock_guard<boost::mutex> g(_mutex);
  _domMkts.clear();
} // QueryGetDomFaresHistorical::clearDomMkts()

///////////////////////////////////////////////////////////
//
//  QueryGetDomMarkets_Fare
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetDomMarkets_Fare::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetDomFares.GetDomMarkets_Fare"));
std::string QueryGetDomMarkets_Fare::_baseSQL;
bool QueryGetDomMarkets_Fare::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetDomMarkets_Fare> g_GetDomMarkets_Fare;

const char*
QueryGetDomMarkets_Fare::getQueryName() const
{
  return "GETDOMINTMARKETS";
}

void
QueryGetDomMarkets_Fare::initialize()
{
  if (!_isInitialized)
  {
    QueryGetDomMarkets_FareSQLStatement<QueryGetDomMarkets_Fare> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETDOMINTMARKETS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetDomMarkets_Fare::getDomMarkets(std::vector<LocCode>& domMkts)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    domMkts.push_back(
        QueryGetDomMarkets_FareSQLStatement<QueryGetDomMarkets_Fare>::mapRowToMarket(row));
  }
  LOG4CXX_INFO(_logger,
               "GETDOMINTMARKETS: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                              << stopCPU() << ")");
  res.freeResult();
} // QueryGetDomMarkets_Fare::getDomMarkets()

///////////////////////////////////////////////////////////
//
//  QueryGetDomMarkets_FareHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetDomMarkets_FareHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetDomFaresHistorical.GetDomMarkets_FareHistorical"));
std::string QueryGetDomMarkets_FareHistorical::_baseSQL;
bool QueryGetDomMarkets_FareHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetDomMarkets_FareHistorical> g_GetDomMarkets_FareHistorical;

const char*
QueryGetDomMarkets_FareHistorical::getQueryName() const
{
  return "GETDOMINTMARKETSHISTORICAL";
}

void
QueryGetDomMarkets_FareHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetDomMarkets_FareHistoricalSQLStatement<QueryGetDomMarkets_FareHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETDOMINTMARKETSHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetDomMarkets_FareHistorical::getDomMarkets(std::vector<MarketAndDate>& domMkts)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  MarketAndDate md;

  resetSQL();

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  domMkts.reserve(res.numRows());
  while ((row = res.nextRow()))
  {
    domMkts.push_back(QueryGetDomMarkets_FareHistoricalSQLStatement<
        QueryGetDomMarkets_FareHistorical>::mapRowToMarket(row, md));
  }
  LOG4CXX_INFO(_logger,
               "GETDOMINTMARKETSHISTORICAL: NumRows = " << res.numRows() << " Time = "
                                                        << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetDomMarkets_FareHistorical::getDomMarkets()

///////////////////////////////////////////////////////////
//
//  QueryGetIntFares
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetIntFares::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetDomFares.GetIntFares"));
std::string QueryGetIntFares::_baseSQL;
bool QueryGetIntFares::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetIntFares> g_GetIntFares;

const char*
QueryGetIntFares::getQueryName() const
{
  return "GETINTFARES";
}

void
QueryGetIntFares::initialize()
{
  if (UNLIKELY(!_isInitialized))
  {
    QueryGetIntFaresSQLStatement<QueryGetIntFares> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETINTFARES");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetIntFares::getIntFares(std::vector<const tse::FareInfo*>& lstFI,
                              const LocCode& market1,
                              const LocCode& market2,
                              const CarrierCode& carrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  std::vector<CarrierCode> carriers;
  carriers.push_back(carrier);

  substParm(market1, 1);
  substParm(market2, 2);
  substParm(3, carriers);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    lstFI.push_back(QueryGetIntFaresSQLStatement<QueryGetIntFares>::mapRowToFareInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETINTFARES: NumRows " << res.numRows() << " Time = " << stopTimer() << " ("
                                       << stopCPU() << ")");
  res.freeResult();
} // QueryGetIntFares::getIntFares(Single Carrier)

void
QueryGetIntFares::getIntFares(std::vector<const tse::FareInfo*>& lstFI,
                              const LocCode& market1,
                              const LocCode& market2,
                              const std::vector<CarrierCode>& carriers)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(market1, 1);
  substParm(market2, 2);
  substParm(3, carriers);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    lstFI.push_back(QueryGetIntFaresSQLStatement<QueryGetIntFares>::mapRowToFareInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETCXRINTFARES: NumRows " << res.numRows() << " Time = " << stopTimer() << " ("
                                          << stopCPU() << ")");
  res.freeResult();
} // QueryGetIntFares::getIntFares(Multiple Carriers)

///////////////////////////////////////////////////////////
//
//  QueryGetIntFaresHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetIntFaresHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetDomFares.GetIntFaresHistorical"));
std::string QueryGetIntFaresHistorical::_baseSQL;
bool QueryGetIntFaresHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetIntFaresHistorical> g_GetIntFaresHistorical;

const char*
QueryGetIntFaresHistorical::getQueryName() const
{
  return "GETINTFARESHISTORICAL";
}

void
QueryGetIntFaresHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetIntFaresHistoricalSQLStatement<QueryGetIntFaresHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETINTFARESHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetIntFaresHistorical::getIntFares(std::vector<const tse::FareInfo*>& lstFI,
                                        const LocCode& market1,
                                        const LocCode& market2,
                                        const CarrierCode& carrier,
                                        const DateTime& startDate,
                                        const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  std::vector<CarrierCode> carriers;
  carriers.push_back(carrier);

  substParm(market1, 1);
  substParm(market2, 2);
  substParm(3, carriers);
  substParm(4, startDate);
  substParm(5, endDate);
  substParm(market1, 6);
  substParm(market2, 7);
  substParm(8, carriers);
  substParm(9, startDate);
  substParm(10, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    lstFI.push_back(
        QueryGetIntFaresHistoricalSQLStatement<QueryGetIntFaresHistorical>::mapRowToFareInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETINTFARESHISTORICAL: NumRows " << res.numRows() << " Time = " << stopTimer()
                                                 << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetIntFaresHistorical::getIntFares(Single Carrier)

void
QueryGetIntFaresHistorical::getIntFares(std::vector<const tse::FareInfo*>& lstFI,
                                        const LocCode& market1,
                                        const LocCode& market2,
                                        const std::vector<CarrierCode>& carriers,
                                        const DateTime& startDate,
                                        const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(market1, 1);
  substParm(market2, 2);
  substParm(3, carriers);
  substParm(4, startDate);
  substParm(5, endDate);
  substParm(market1, 6);
  substParm(market2, 7);
  substParm(8, carriers);
  substParm(9, startDate);
  substParm(10, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    lstFI.push_back(
        QueryGetIntFaresHistoricalSQLStatement<QueryGetIntFaresHistorical>::mapRowToFareInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETCXRINTFARESHISTORICAL: NumRows " << res.numRows() << " Time = " << stopTimer()
                                                    << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetIntFaresHistorical::getIntFares(Multiple Carriers)

///////////////////////////////////////////////////////////
//
//  QueryGetAllFaresHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllFaresHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetDomFares.GetAllFaresHistorical"));
std::string QueryGetAllFaresHistorical::_baseSQL;
bool QueryGetAllFaresHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllFaresHistorical> g_GetAllFaresHistorical;

const char*
QueryGetAllFaresHistorical::getQueryName() const
{
  return "GETALLFARESHISTORICAL";
}

void
QueryGetAllFaresHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllFaresHistoricalSQLStatement<QueryGetAllFaresHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLFARESHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllFaresHistorical::getAllFares(std::vector<const tse::FareInfo*>& lstFI,
                                        const LocCode& market1,
                                        const LocCode& market2,
                                        const CarrierCode& carrier,
                                        const DateTime& startDate,
                                        const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  std::vector<CarrierCode> carriers;
  carriers.push_back(carrier);

  resetSQL();
  for (int idx = 0; idx < 4; ++idx)
  {
    substParm(market1, 5 * idx + 1);
    substParm(market2, 5 * idx + 2);
    substParm(5 * idx + 3, carriers);
    substParm(5 * idx + 4, startDate);
    substParm(5 * idx + 5, endDate);
  }
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    lstFI.push_back(
        QueryGetAllFaresHistoricalSQLStatement<QueryGetAllFaresHistorical>::mapRowToFareInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLFARESHISTORICAL: NumRows " << res.numRows() << " Time = " << stopTimer()
                                                 << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetAllFaresHistorical::getAllFares(Single Carrier)

void
QueryGetAllFaresHistorical::getAllFares(std::vector<const tse::FareInfo*>& lstFI,
                                        const LocCode& market1,
                                        const LocCode& market2,
                                        const std::vector<CarrierCode>& carriers,
                                        const DateTime& startDate,
                                        const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();
  for (int idx = 0; idx < 4; ++idx)
  {
    substParm(market1, 5 * idx + 1);
    substParm(market2, 5 * idx + 2);
    substParm(5 * idx + 3, carriers);
    substParm(5 * idx + 4, startDate);
    substParm(5 * idx + 5, endDate);
  }
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    lstFI.push_back(
        QueryGetAllFaresHistoricalSQLStatement<QueryGetAllFaresHistorical>::mapRowToFareInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLFARESHISTORICAL: NumRows " << res.numRows() << " Time = " << stopTimer()
                                                 << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetAllFaresHistorical::getAllFares(Multi Carrier)
}
