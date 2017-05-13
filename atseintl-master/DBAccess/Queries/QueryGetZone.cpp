//----------------------------------------------------------------------------
//  File:           QueryGetZone.cpp
//  Description:    QueryGetZone
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

#include "DBAccess/Queries/QueryGetZone.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetZoneSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetZone::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetZone"));
std::string QueryGetZone::_baseSQL;
bool QueryGetZone::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetZone> g_GetZone;

const char*
QueryGetZone::getQueryName() const
{
  return "GETZONE";
}

void
QueryGetZone::initialize()
{
  if (!_isInitialized)
  {
    QueryGetZoneSQLStatement<QueryGetZone> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETZONE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()


namespace
{
  void sortUniformZone(ZoneInfo* zone)
  {
    typedef std::vector<ZoneInfo::ZoneSeg> ZoneLocSet;

    if (zone->isUniform())
    {
      ZoneLocSet& segs(zone->sets().front());
      if (zone->sets().size() > 1)
      {
        // Collapse all zone segments into single set -
        // multiple sets are meaningless in case of uniform zone.
        for (auto setItr = ++zone->sets().begin(); setItr != zone->sets().end(); ++setItr)
          segs.insert(segs.end(), setItr->begin(), setItr->end());
        zone->sets().erase(++zone->sets().begin(), zone->sets().end());
      }
      // Sort zone segments/locations to enable binary search.
      std::sort(segs.begin(), segs.end(), ZoneSegCmpByLoc());
    }
  }
}

void
QueryGetZone::findZone(std::vector<const tse::ZoneInfo*>& zones,
                       VendorCode& vendor,
                       Zone& zone,
                       Indicator zoneType)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, vendor);
  substParm(2, zone);
  substParm(3, zoneType);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  ZoneInfo* zoneInfo = nullptr;
  ZoneInfo* prevZone = nullptr;
  DateTime prevDate;
  int prevSet = 0;
  while ((row = res.nextRow()))
  {
    zoneInfo =
        QueryGetZoneSQLStatement<QueryGetZone>::mapRowToZone(row, prevZone, prevDate, prevSet);
    if (prevZone != zoneInfo)
    {
      if (prevZone != nullptr)
        sortUniformZone(prevZone);

      prevZone = zoneInfo;
      zones.push_back(zoneInfo);
    }
  }
  if (zoneInfo != nullptr)
    sortUniformZone(zoneInfo);

  LOG4CXX_INFO(_logger,
               "GETZONE: NumRows = " << res.numRows() << " Time = " << stopTimer() << " msecs");
  res.freeResult();
} // findZone()

///////////////////////////////////////////////////////////
//
//  QueryGetZoneHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetZoneHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetZoneHistorical"));
std::string QueryGetZoneHistorical::_baseSQL;
bool QueryGetZoneHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetZoneHistorical> g_GetZoneHistorical;

const char*
QueryGetZoneHistorical::getQueryName() const
{
  return "GETZONEHISTORICAL";
}

void
QueryGetZoneHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetZoneHistoricalSQLStatement<QueryGetZoneHistorical> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETZONEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetZoneHistorical::findZone(std::vector<const tse::ZoneInfo*>& zones,
                                 VendorCode& vendor,
                                 Zone& zone,
                                 Indicator zoneType,
                                 const DateTime& startDate,
                                 const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, vendor);
  substParm(2, zone);
  substParm(3, zoneType);
  substParm(4, startDate);
  substParm(5, endDate);
  substParm(6, vendor);
  substParm(7, zone);
  substParm(8, zoneType);
  substParm(9, startDate);
  substParm(10, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  ZoneInfo* zoneInfo = nullptr;
  ZoneInfo* prevZone = nullptr;
  DateTime prevDate;
  int prevSet = 0;
  while ((row = res.nextRow()))
  {
    zoneInfo = QueryGetZoneHistoricalSQLStatement<QueryGetZoneHistorical>::mapRowToZone(
        row, prevZone, prevDate, prevSet);
    if (prevZone != zoneInfo)
    {
      if (prevZone != nullptr)
        sortUniformZone(prevZone);

      prevZone = zoneInfo;
      zones.push_back(zoneInfo);
    }
  }
  if (zoneInfo != nullptr)
    sortUniformZone(zoneInfo);

  LOG4CXX_INFO(_logger,
               "GETZONE: NumRows = " << res.numRows() << " Time = " << stopTimer() << " msecs");
  res.freeResult();
} // findZone()

}
