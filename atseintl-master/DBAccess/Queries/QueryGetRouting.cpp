//----------------------------------------------------------------------------
//  File:           QueryGetRouting.cpp
//  Description:    QueryGetRouting
//  Created:        8/24/2006
// Authors:         Mike Lillis
//
//  Updates:
//
// � 2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetRouting.h"

#include "Common/FallbackUtil.h"
#include "Common/Global.h"
#include "Common/RoutingCycleDetector.h"
#include "Common/RoutingDbValidator.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetRoutingSQLStatement.h"

#include <algorithm>

namespace tse
{
FIXEDFALLBACK_DECL(detectRoutingCycles);
FIXEDFALLBACK_DECL(routingCycleDetectorHardening)

log4cxx::LoggerPtr
QueryGetRouting::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetRouting"));
std::string QueryGetRouting::_baseSQL;
log4cxx::LoggerPtr
QueryGetRoutingRest::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetRouting.GetRoutingRest"));
std::string QueryGetRoutingRest::_baseSQL;

bool QueryGetRoutingRest::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetRoutingRest> g_GetRoutingRest;
bool QueryGetRouting::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetRouting> g_GetRouting;

const char*
QueryGetRouting::getQueryName() const
{
  return "GETROUTING";
};

void
QueryGetRouting::initialize()
{
  if (!_isInitialized)
  {
    QueryGetRoutingSQLStatement<QueryGetRouting> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETROUTING");
    substTableDef(&_baseSQL);

    QueryGetRoutingRest::initialize();
    _isInitialized = true;
  }
} // initialize()

static bool
validateRoutingOld(const tse::Routing* rtg)
{
  RoutingCycleDetector rcd(rtg->rmaps());
  return rcd.validate();
}

inline static bool
validateRouting(const tse::Routing* rtg)
{
  return RoutingDbValidator::validate(rtg->rmaps());
}

static void
filterRoutings(std::vector<tse::Routing*>& routings)
{
  decltype(routings.begin()) firstInvalid;
  if (fallback::fixed::routingCycleDetectorHardening())
    firstInvalid = std::stable_partition(routings.begin(), routings.end(), validateRoutingOld);
  else
    firstInvalid = std::stable_partition(routings.begin(), routings.end(), validateRouting);

  if (firstInvalid != routings.end())
  {
    for (auto it = firstInvalid; it != routings.end(); ++it)
      delete *it;
    routings.erase(firstInvalid, routings.end());
  }
}

void
QueryGetRouting::findRouting(std::vector<tse::Routing*>& routings,
                             const VendorCode& vendor,
                             const CarrierCode& carrier,
                             int& routingTariff,
                             const RoutingNumber& routing)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  char ruleTarStr[15];
  sprintf(ruleTarStr, "%d", routingTariff);

  substParm(vendor, 1);
  substParm(carrier, 2);
  substParm(ruleTarStr, 3);
  substParm(routing, 4);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::Routing* curRouting = nullptr;
  tse::Routing* routingPrev = nullptr;
  while ((row = res.nextRow()))
  {
    curRouting = QueryGetRoutingSQLStatement<QueryGetRouting>::mapRowToRouting(row, routingPrev);
    if (curRouting != routingPrev)
      routings.push_back(curRouting);

    routingPrev = curRouting;
  }
  LOG4CXX_INFO(_logger,
               "GETROUTING: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                        << stopCPU() << ")");
  res.freeResult();

  // Filter out routings with cycles
  if (!fallback::fixed::detectRoutingCycles())
    filterRoutings(routings);

  // Now get Routing Restrictions;
  QueryGetRoutingRest SQLRoutingRest(_dbAdapt);

  std::vector<tse::Routing*>::iterator i;
  for (i = routings.begin(); i != routings.end(); i++)
  {
    SQLRoutingRest.getRoutingRest(*i);
  }
} // QueryGetRouting::findRouting()

///////////////////////////////////////////////////////////
//  QueryGetRoutingHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetRoutingHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetRoutingHistorical"));
std::string QueryGetRoutingHistorical::_baseSQL;
log4cxx::LoggerPtr
QueryGetRoutingRestHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetRoutingHist.GetRoutingRestHistorical"));
std::string QueryGetRoutingRestHistorical::_baseSQL;

bool QueryGetRoutingRestHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetRoutingRestHistorical> g_GetRoutingRestHistorical;
bool QueryGetRoutingHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetRoutingHistorical> g_GetRoutingHistorical;

const char*
QueryGetRoutingHistorical::getQueryName() const
{
  return "GETROUTINGHISTORICAL";
};

void
QueryGetRoutingHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetRoutingHistoricalSQLStatement<QueryGetRoutingHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETROUTINGHISTORICAL");
    substTableDef(&_baseSQL);

    QueryGetRoutingRestHistorical::initialize();
    _isInitialized = true;
  }
} // initialize()

void
QueryGetRoutingHistorical::findRouting(std::vector<tse::Routing*>& routings,
                                       const VendorCode& vendor,
                                       const CarrierCode& carrier,
                                       int& routingTariff,
                                       const RoutingNumber& routing,
                                       const DateTime& startDate,
                                       const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  char ruleTarStr[15];
  sprintf(ruleTarStr, "%d", routingTariff);

  substParm(vendor, 1);
  substParm(carrier, 2);
  substParm(ruleTarStr, 3);
  substParm(routing, 4);
  substParm(5, startDate);
  substParm(6, endDate);
  substParm(7, endDate);
  substParm(vendor, 8);
  substParm(carrier, 9);
  substParm(ruleTarStr, 10);
  substParm(routing, 11);
  substParm(12, startDate);
  substParm(13, endDate);
  substParm(14, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::Routing* curRouting = nullptr;
  tse::Routing* routingPrev = nullptr;
  while ((row = res.nextRow()))
  {
    curRouting = QueryGetRoutingHistoricalSQLStatement<QueryGetRoutingHistorical>::mapRowToRouting(
        row, routingPrev);
    if (curRouting != routingPrev)
      routings.push_back(curRouting);

    routingPrev = curRouting;
  }
  LOG4CXX_INFO(_logger,
               "GETROUTINGHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                  << " (" << stopCPU() << ")");
  res.freeResult();

  // Filter out routings with cycles
  if (!fallback::fixed::detectRoutingCycles())
    filterRoutings(routings);

  // Now get Routing Restrictions;
  QueryGetRoutingRestHistorical SQLRoutingRest(_dbAdapt);

  std::vector<Routing*>::iterator i;
  for (i = routings.begin(); i != routings.end(); i++)
  {
    SQLRoutingRest.getRoutingRest(*i);
  }
} // QueryGetRoutingHistorical::findRouting()

///////////////////////////////////////////////////////////
//  QueryGetRoutingRest
///////////////////////////////////////////////////////////
const char*
QueryGetRoutingRest::getQueryName() const
{
  return "GETROUTINGREST";
}

void
QueryGetRoutingRest::initialize()
{
  if (!_isInitialized)
  {
    QueryGetRoutingRestSQLStatement<QueryGetRoutingRest> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETROUTINGREST");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetRoutingRest::getRoutingRest(Routing* a_pRouting)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  char ruleTarStr[15];
  sprintf(ruleTarStr, "%d", a_pRouting->routingTariff());

  resetSQL();

  substParm(a_pRouting->vendor(), 1);
  substParm(a_pRouting->carrier(), 2);
  substParm(ruleTarStr, 3);
  substParm(a_pRouting->routing(), 4);
  substParm(5, a_pRouting->effDate());
  substParm(6, a_pRouting->createDate());
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  RoutingRestriction* curRouting = nullptr;
  while ((row = res.nextRow()))
  {
    curRouting =
        QueryGetRoutingRestSQLStatement<QueryGetRoutingRest>::mapRowToRoutingRestriction(row);
    a_pRouting->rests().push_back(curRouting);
  }
  LOG4CXX_INFO(_logger,
               "GETROUTINGREST: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                            << stopCPU() << ")");
  res.freeResult();
} // QueryGetRoutingRest::getRoutingRest()

///////////////////////////////////////////////////////////
//  QueryGetRoutingRestHistorical
///////////////////////////////////////////////////////////
const char*
QueryGetRoutingRestHistorical::getQueryName() const
{
  return "GETROUTINGRESTHISTORICAL";
}

void
QueryGetRoutingRestHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetRoutingRestHistoricalSQLStatement<QueryGetRoutingRestHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETROUTINGRESTHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetRoutingRestHistorical::getRoutingRest(Routing* a_pRouting)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  char ruleTarStr[15];
  sprintf(ruleTarStr, "%d", a_pRouting->routingTariff());

  resetSQL();

  substParm(a_pRouting->vendor(), 1);
  substParm(a_pRouting->carrier(), 2);
  substParm(ruleTarStr, 3);
  substParm(a_pRouting->routing(), 4);
  substParm(5, a_pRouting->effDate());
  substParm(6, a_pRouting->createDate());
  substParm(a_pRouting->vendor(), 7);
  substParm(a_pRouting->carrier(), 8);
  substParm(ruleTarStr, 9);
  substParm(a_pRouting->routing(), 10);
  substParm(11, a_pRouting->effDate());
  substParm(12, a_pRouting->createDate());
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  RoutingRestriction* curRouting = nullptr;
  while ((row = res.nextRow()))
  {
    curRouting = QueryGetRoutingRestHistoricalSQLStatement<
        QueryGetRoutingRestHistorical>::mapRowToRoutingRestriction(row);
    a_pRouting->rests().push_back(curRouting);
  }
  LOG4CXX_INFO(_logger,
               "GETROUTINGRESTHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetRoutingRestHistorical::getRoutingRest()
}
