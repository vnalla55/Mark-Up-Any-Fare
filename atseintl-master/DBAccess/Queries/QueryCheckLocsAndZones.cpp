//----------------------------------------------------------------------------
//          File:           QueryCheckLocsAndZones.cpp
//          Description:    QueryCheckLocsAndZones
//          Created:        3/2/2006
// Authors:         Mike Lillis
//
//          Updates:
//
//     � 2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#include "DBAccess/Queries/QueryCheckLocsAndZones.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryCheckLocsAndZonesSQLStatement.h"

namespace tse
{


///////////////////////////////////////////////////////////
//  QueryGetSITAAddonScore
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetSITAAddonScore::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetSITAAddonScore"));
std::string QueryGetSITAAddonScore::_baseSQL;
bool QueryGetSITAAddonScore::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSITAAddonScore> g_GetSITAAddonScore;

log4cxx::LoggerPtr
QueryGetZoneLocTypes::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetSITAAddonScore.GetZoneLocTypes"));
std::string QueryGetZoneLocTypes::_baseSQL;
bool QueryGetZoneLocTypes::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetZoneLocTypes> g_GetZoneLocTypes;

log4cxx::LoggerPtr
QueryGetMktCtyZone::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetSITAAddonScore.GetMktCtyZone"));
std::string QueryGetMktCtyZone::_baseSQL;
bool QueryGetMktCtyZone::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMktCtyZone> g_GetMktCtyZone;

log4cxx::LoggerPtr
QueryGetMktStateZone::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetSITAAddonScore.GetMktStateZone"));
std::string QueryGetMktStateZone::_baseSQL;
bool QueryGetMktStateZone::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMktStateZone> g_GetMktStateZone;

log4cxx::LoggerPtr
QueryGetMktNationZone::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetSITAAddonScore.GetMktNationZone"));
std::string QueryGetMktNationZone::_baseSQL;
bool QueryGetMktNationZone::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMktNationZone> g_GetMktNationZone;

log4cxx::LoggerPtr
QueryGetMktSubAreaZone::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetSITAAddonScore.GetMktSubAreaZone"));
std::string QueryGetMktSubAreaZone::_baseSQL;
bool QueryGetMktSubAreaZone::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMktSubAreaZone> g_GetMktSubAreaZone;

log4cxx::LoggerPtr
QueryGetMktAreaZone::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetSITAAddonScore.GetMktAreaZone"));
std::string QueryGetMktAreaZone::_baseSQL;
bool QueryGetMktAreaZone::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMktAreaZone> g_GetMktAreaZone;

log4cxx::LoggerPtr
QueryGetMktZoneZone::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetSITAAddonScore.GetMktZoneZone"));
std::string QueryGetMktZoneZone::_baseSQL;
bool QueryGetMktZoneZone::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMktZoneZone> g_GetMktZoneZone;

static inline bool
isReserveZone(const char* zoneType)
{
  return strcmp(zoneType, "R") == 0 || strcmp(zoneType, "T") == 0;
}

const char*
QueryGetSITAAddonScore::getQueryName() const
{
  return "GETSITAADDONSCORE";
};

void
QueryGetSITAAddonScore::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSITAAddonScoreSQLStatement<QueryGetSITAAddonScore> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSITAADDONSCORE");
    substTableDef(&_baseSQL);

    QueryGetZoneLocTypes::initialize();
    QueryGetMktCtyZone::initialize();
    QueryGetMktStateZone::initialize();
    QueryGetMktNationZone::initialize();
    QueryGetMktSubAreaZone::initialize();
    QueryGetMktAreaZone::initialize();
    QueryGetMktZoneZone::initialize();
    _isInitialized = true;
  }
} // QueryGetSITAAddonScore::initialize()

void
QueryGetSITAAddonScore::evaluateAddonZoneSITA(LocCode& loc,
                                              VendorCode& vendor,
                                              CarrierCode& carrier,
                                              std::vector<AddonZoneInfo*>& validZones)
{
  static const char* SITA_ADDON_SUBAREAS[] = { "00911", "00912", "00913", "00921",
                                               "00922", "00923", "00931", "00932" };
  static constexpr int N_OF_SUBAREAS = sizeof(SITA_ADDON_SUBAREAS) / sizeof(const char*);

  std::string sitaSubArea = "";

  for (const char** subarea = SITA_ADDON_SUBAREAS; subarea < SITA_ADDON_SUBAREAS + N_OF_SUBAREAS;
       subarea++)
    if (isLocInZone(loc, stringToInteger(*subarea, __LINE__), RESERVED, SITA_VENDOR_CODE))
    {
      sitaSubArea = *subarea;
      break;
    }

  Row* row;
  DBResultSet res(_dbAdapt);

  std::map<std::string, int> zoneScore;

  substParm(loc, 1);
  substParm(carrier, 2);
  substParm(loc, 3);
  substParm(carrier, 4);
  substParm(loc, 5);
  substParm(carrier, 6);
  substParm(loc, 7);
  substParm(carrier, 8);
  substParm(loc, 9);
  substParm(carrier, 10);
  substParm(sitaSubArea, 11);
  substParm(carrier, 12);
  substCurrentDate();
  res.executeQuery(this);

  int prevZone = 0;
  int prevTariff = 0;
  int prevScore = 0;

  struct SITAAddonScoreResult result;
  while ((row = res.nextRow()))
  { // keep highest score for each zone/tariff
    QueryGetSITAAddonScoreSQLStatement<QueryGetSITAAddonScore>::mapRowToSITAAddonScoreResult(
        row, result);
    int zone = result.zone;
    int tariff = result.addonTarriff;
    int score = result.score;

    if (prevZone != zone || prevTariff != tariff)
    {
      prevZone = zone;
      prevTariff = tariff;
      prevScore = score < 0 ? score : 0;
    }
    if (prevScore <= 0 && score > abs(prevScore))
    {
      prevScore = score;
      AddonZoneInfo* addOn = new AddonZoneInfo;
      addOn->vendor() = SITA_VENDOR_CODE;
      addOn->carrier() = carrier;
      addOn->zone() = zone;
      addOn->fareTariff() = tariff;

      addOn->inclExclInd() = result.inclExclInd;

      addOn->market().locType() = result.locType;
      addOn->market().loc() = result.loc;
      addOn->zone() = result.zone;

      addOn->createDate() = result.createDate;
      addOn->expireDate() = result.expireDate;
      addOn->effDate() = result.effDate;
      addOn->discDate() = result.discDate;

      validZones.push_back(addOn);
    } // if
  } // while

  res.freeResult();
} // evaluateAddonZoneSITA()

bool
QueryGetSITAAddonScore::isLocInZone(const tse::LocCode& loc,
                                    int zone,
                                    char zoneType,
                                    const VendorCode& vendor)
{
  int score = evaluateZone(loc, zone, zoneType, vendor, INCLUDE_LOC);
  if (score > 0)
  {
    int scoreExcl = evaluateZone(loc, zone, zoneType, vendor, EXCLUDE_LOC);
    if (scoreExcl > score)
      return false;
    else
      return true;
  } // if score > 0

  return false;
} // QueryGetSITAAddonScore::isLocInZone()

int
QueryGetSITAAddonScore::stringToInteger(const char* stringVal, int lineNumber)
{
  if (stringVal == nullptr)
  {
    LOG4CXX_FATAL(_logger,
                  "stringToInteger - Null pointer to int data. LineNumber: " << lineNumber);
    throw std::runtime_error("Null pointer to int data");
  }
  else if (*stringVal == '-' || *stringVal == '+')
  {
    if (stringVal[1] < '0' || stringVal[1] > '9')
      return 0;
  }
  else if (*stringVal < '0' || *stringVal > '9')
  {
    return 0;
  }
  return atoi(stringVal);
} // QueryGetSITAAddonScore::stringToInteger()

int
QueryGetSITAAddonScore::evaluateZone(const tse::LocCode& loc,
                                     int zone,
                                     char zoneType,
                                     const VendorCode& vendor,
                                     IncludeExcludeLoc inclExclSpec)
{
  // Parms for all the subqueries
  char zoneStr[] = "0000000";
  char znStr[15];
  sprintf(znStr, "%d", zone);
  strcpy(&zoneStr[7 - strlen(znStr)], znStr);

  char zoneTypeStr[15];
  sprintf(zoneTypeStr, "%c", zoneType);

  std::string inclExcl;
  switch (inclExclSpec)
  {
  case INCLUDE_LOC:
    inclExcl = "I";
    break;
  case EXCLUDE_LOC:
    inclExcl = "E";
    break;
  }

  // Receptacles for a couple of subqueries
  std::vector<std::string> locTypes;
  std::vector<int> zoneZone;

  QueryGetZoneLocTypes SQLZoneLocTypes(_dbAdapt);
  locTypes = SQLZoneLocTypes.getLocTypes(zoneStr, zoneTypeStr, inclExcl, vendor);

  int score = 0;
  int scoreCall = 0;
  for (const auto& locType : locTypes)
  {
    if (locType.c_str()[0] == 'C')
    {
      QueryGetMktCtyZone SQLMktCtyZone(_dbAdapt);
      scoreCall = SQLMktCtyZone.getScore(zoneTypeStr, inclExcl, loc, zoneStr, vendor);
    }
    else if (locType.c_str()[0] == 'S')
    {
      QueryGetMktStateZone SQLMktStateZone(_dbAdapt);
      scoreCall = SQLMktStateZone.getScore(zoneTypeStr, inclExcl, loc, zoneStr, vendor);
    }
    else if (locType.c_str()[0] == 'N')
    {
      QueryGetMktNationZone SQLMktNationZone(_dbAdapt);
      scoreCall = SQLMktNationZone.getScore(zoneTypeStr, inclExcl, loc, zoneStr, vendor);
    }
    else if (locType.c_str()[0] == '*')
    {
      QueryGetMktSubAreaZone SQLMktSubAreaZone(_dbAdapt);
      scoreCall = SQLMktSubAreaZone.getScore(zoneTypeStr, inclExcl, loc, zoneStr, vendor);
    }
    else if (locType.c_str()[0] == 'A')
    {
      QueryGetMktAreaZone SQLMktAreaZone(_dbAdapt);
      scoreCall = SQLMktAreaZone.getScore(zoneTypeStr, inclExcl, loc, zoneStr, vendor);
    }
    else if (locType.c_str()[0] == 'Z')
    {
      QueryGetMktZoneZone SQLMktZoneZone(_dbAdapt);
      zoneZone = SQLMktZoneZone.getZones(zoneTypeStr, inclExcl, zoneStr, vendor);

      if (zoneType == 'U' || zoneType == 'T')
        zoneType = 'R'; // User defined zones are made up of 'R' type zones

      size_t j;
      for (j = 0; j < zoneZone.size(); j++)
      {
        if ((scoreCall = isLocInZone(loc, zoneZone[j], zoneType, vendor)))
        {
          break;
        }
      } // for
    } // Zone

    score = score < scoreCall ? scoreCall : score;
  } // for()
  return score;
} // QueryGetSITAAddonScore::evaluateZone()

///////////////////////////////////////////////////////////
//  QueryGetSITAAddonScoreHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetSITAAddonScoreHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetSITAAddonScoreHistorical"));
std::string QueryGetSITAAddonScoreHistorical::_baseSQL;
bool QueryGetSITAAddonScoreHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSITAAddonScoreHistorical> g_GetSITAAddonScoreHistorical;

log4cxx::LoggerPtr
QueryGetZoneLocTypesHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetSITAAddonScoreHistorical.GetZoneLocTypesHistorical"));
std::string QueryGetZoneLocTypesHistorical::_baseSQL;
bool QueryGetZoneLocTypesHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetZoneLocTypesHistorical> g_GetZoneLocTypesHistorical;

log4cxx::LoggerPtr
QueryGetMktCtyZoneHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetSITAAddonScoreHistorical.GetMktCtyZoneHistorical"));
std::string QueryGetMktCtyZoneHistorical::_baseSQL;
bool QueryGetMktCtyZoneHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMktCtyZoneHistorical> g_GetMktCtyZoneHistorical;

log4cxx::LoggerPtr
QueryGetMktStateZoneHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetSITAAddonScoreHistorical.GetMktStateZoneHistorical"));
std::string QueryGetMktStateZoneHistorical::_baseSQL;
bool QueryGetMktStateZoneHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMktStateZoneHistorical> g_GetMktStateZoneHistorical;

log4cxx::LoggerPtr
QueryGetMktNationZoneHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetSITAAddonScoreHistorical.GetMktNationZoneHistorical"));
std::string QueryGetMktNationZoneHistorical::_baseSQL;
bool QueryGetMktNationZoneHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMktNationZoneHistorical> g_GetMktNationZoneHistorical;

log4cxx::LoggerPtr
QueryGetMktSubAreaZoneHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetSITAAddonScoreHistorical.GetMktSubAreaZoneHistorical"));
std::string QueryGetMktSubAreaZoneHistorical::_baseSQL;
bool QueryGetMktSubAreaZoneHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMktSubAreaZoneHistorical> g_GetMktSubAreaZoneHistorical;

log4cxx::LoggerPtr
QueryGetMktAreaZoneHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetSITAAddonScoreHistorical.GetMktAreaZoneHistorical"));
std::string QueryGetMktAreaZoneHistorical::_baseSQL;
bool QueryGetMktAreaZoneHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMktAreaZoneHistorical> g_GetMktAreaZoneHistorical;

log4cxx::LoggerPtr
QueryGetMktZoneZoneHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetSITAAddonScoreHistorical.GetMktZoneZoneHistorical"));
std::string QueryGetMktZoneZoneHistorical::_baseSQL;
bool QueryGetMktZoneZoneHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMktZoneZoneHistorical> g_GetMktZoneZoneHistorical;

const char*
QueryGetSITAAddonScoreHistorical::getQueryName() const
{
  return "GETSITAADDONSCOREHISTORICAL";
};

void
QueryGetSITAAddonScoreHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSITAAddonScoreHistoricalSQLStatement<QueryGetSITAAddonScoreHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSITAADDONSCOREHISTORICAL");
    substTableDef(&_baseSQL);

    QueryGetZoneLocTypesHistorical::initialize();
    QueryGetMktCtyZoneHistorical::initialize();
    QueryGetMktStateZoneHistorical::initialize();
    QueryGetMktNationZoneHistorical::initialize();
    QueryGetMktSubAreaZoneHistorical::initialize();
    QueryGetMktAreaZoneHistorical::initialize();
    QueryGetMktZoneZoneHistorical::initialize();
    _isInitialized = true;
  }
} // QueryGetSITAAddonScoreHistorical::initialize()

void
QueryGetSITAAddonScoreHistorical::evaluateAddonZoneSITA(LocCode& loc,
                                                        VendorCode& vendor,
                                                        CarrierCode& carrier,
                                                        std::vector<AddonZoneInfo*>& validZones,
                                                        const DateTime& startDate,
                                                        const DateTime& endDate)
{
  static const char* SITA_ADDON_SUBAREAS[] = { "00911", "00912", "00913", "00921",
                                               "00922", "00923", "00931", "00932" };
  static constexpr int N_OF_SUBAREAS = sizeof(SITA_ADDON_SUBAREAS) / sizeof(const char*);

  std::string sitaSubArea = "";

  for (const char** subarea = SITA_ADDON_SUBAREAS; subarea < SITA_ADDON_SUBAREAS + N_OF_SUBAREAS;
       subarea++)
    if (isLocInZone(loc,
                    stringToInteger(*subarea, __LINE__),
                    RESERVED,
                    SITA_VENDOR_CODE,
                    startDate,
                    endDate))
    {
      sitaSubArea = *subarea;
      break;
    }

  Row* row;
  DBResultSet res(_dbAdapt);

  std::map<std::string, int> zoneScore;

  // Historical Part
  substParm(1, loc);
  substParm(2, carrier);
  substParm(3, startDate);
  substParm(4, endDate);
  substParm(5, endDate);
  substParm(6, startDate);
  substParm(7, endDate);
  substParm(8, endDate);
  substParm(9, loc);
  substParm(10, carrier);
  substParm(11, startDate);
  substParm(12, endDate);
  substParm(13, endDate);
  substParm(14, startDate);
  substParm(15, endDate);
  substParm(16, endDate);
  substParm(17, sitaSubArea);
  substParm(18, carrier);
  substParm(19, startDate);
  substParm(20, endDate);
  substParm(21, endDate);
  substParm(22, loc);
  substParm(23, carrier);
  substParm(24, startDate);
  substParm(25, endDate);
  substParm(26, endDate);
  substParm(27, startDate);
  substParm(28, endDate);
  substParm(29, endDate);
  substParm(30, loc);
  substParm(31, carrier);
  substParm(32, startDate);
  substParm(33, endDate);
  substParm(34, endDate);
  substParm(35, startDate);
  substParm(36, endDate);
  substParm(37, endDate);
  substParm(38, loc);
  substParm(39, carrier);
  substParm(40, startDate);
  substParm(41, endDate);
  substParm(42, endDate);

  // Current Part
  substParm(43, loc);
  substParm(44, carrier);
  substParm(45, startDate);
  substParm(46, endDate);
  substParm(47, endDate);
  substParm(48, startDate);
  substParm(49, endDate);
  substParm(50, endDate);
  substParm(51, loc);
  substParm(52, carrier);
  substParm(53, startDate);
  substParm(54, endDate);
  substParm(55, endDate);
  substParm(56, startDate);
  substParm(57, endDate);
  substParm(58, endDate);
  substParm(59, sitaSubArea);
  substParm(60, carrier);
  substParm(61, startDate);
  substParm(62, endDate);
  substParm(63, endDate);
  substParm(64, loc);
  substParm(65, carrier);
  substParm(66, startDate);
  substParm(67, endDate);
  substParm(68, endDate);
  substParm(69, startDate);
  substParm(70, endDate);
  substParm(71, endDate);
  substParm(72, loc);
  substParm(73, carrier);
  substParm(74, startDate);
  substParm(75, endDate);
  substParm(76, endDate);
  substParm(77, startDate);
  substParm(78, endDate);
  substParm(79, endDate);
  substParm(80, loc);
  substParm(81, carrier);
  substParm(82, startDate);
  substParm(83, endDate);
  substParm(84, endDate);
  res.executeQuery(this);

  int prevZone = 0;
  int prevTariff = 0;
  int prevScore = 0;

  struct SITAAddonScoreResult result;
  while ((row = res.nextRow()))
  { // keep highest score for each zone/tariff
    QueryGetSITAAddonScoreHistoricalSQLStatement<
        QueryGetSITAAddonScoreHistorical>::mapRowToSITAAddonScoreResult(row, result);
    int zone = result.zone;
    int tariff = result.addonTarriff;
    int score = result.score;

    if (prevZone != zone || prevTariff != tariff)
    {
      prevZone = zone;
      prevTariff = tariff;
      prevScore = score < 0 ? score : 0;
    }
    if (prevScore <= 0 && score > abs(prevScore))
    {
      prevScore = score;
      AddonZoneInfo* addOn = new AddonZoneInfo;
      addOn->vendor() = SITA_VENDOR_CODE;
      addOn->carrier() = carrier;
      addOn->zone() = zone;
      addOn->fareTariff() = tariff;

      addOn->inclExclInd() = result.inclExclInd;

      addOn->market().locType() = result.locType;
      addOn->market().loc() = result.loc;
      addOn->zone() = result.zone;

      addOn->createDate() = result.createDate;
      addOn->expireDate() = result.expireDate;
      addOn->effDate() = result.effDate;
      addOn->discDate() = result.discDate;

      validZones.push_back(addOn);
    } // if
  } // while

  res.freeResult();
} // evaluateAddonZoneSITA()

bool
QueryGetSITAAddonScoreHistorical::isLocInZone(const tse::LocCode& loc,
                                              int zone,
                                              char zoneType,
                                              const VendorCode& vendor,
                                              const DateTime& startDate,
                                              const DateTime& endDate)
{
  int score = evaluateZone(loc, zone, zoneType, vendor, INCLUDE_LOC, startDate, endDate);
  if (score > 0)
  {
    int scoreExcl = evaluateZone(loc, zone, zoneType, vendor, EXCLUDE_LOC, startDate, endDate);
    if (scoreExcl > score)
      return false;
    else
      return true;
  } // if score > 0

  return false;
} // QueryGetSITAAddonScoreHistorical::isLocInZone()

int
QueryGetSITAAddonScoreHistorical::stringToInteger(const char* stringVal, int lineNumber)
{
  if (stringVal == nullptr)
  {
    LOG4CXX_FATAL(_logger,
                  "stringToInteger - Null pointer to int data. LineNumber: " << lineNumber);
    throw std::runtime_error("Null pointer to int data");
  }
  else if (*stringVal == '-' || *stringVal == '+')
  {
    if (stringVal[1] < '0' || stringVal[1] > '9')
      return 0;
  }
  else if (*stringVal < '0' || *stringVal > '9')
  {
    return 0;
  }
  return atoi(stringVal);
} // QueryGetSITAAddonScoreHistorical::stringToInteger()

int
QueryGetSITAAddonScoreHistorical::evaluateZone(const tse::LocCode& loc,
                                               int zone,
                                               char zoneType,
                                               const VendorCode& vendor,
                                               IncludeExcludeLoc inclExclSpec,
                                               const DateTime& startDate,
                                               const DateTime& endDate)
{
  // Parms for all the subqueries
  char zoneStr[] = "0000000";
  char znStr[15];
  sprintf(znStr, "%d", zone);
  strcpy(&zoneStr[7 - strlen(znStr)], znStr);

  char zoneTypeStr[15];
  sprintf(zoneTypeStr, "%c", zoneType);

  std::string inclExcl;
  switch (inclExclSpec)
  {
  case INCLUDE_LOC:
    inclExcl = "I";
    break;
  case EXCLUDE_LOC:
    inclExcl = "E";
    break;
  }

  // Receptacles for a couple of subqueries
  std::vector<std::string> locTypes;
  std::vector<int> zoneZone;

  QueryGetZoneLocTypesHistorical SQLZoneLocTypes(_dbAdapt);
  locTypes =
      SQLZoneLocTypes.getLocTypes(zoneStr, zoneTypeStr, inclExcl, vendor, startDate, endDate);

  int score = 0;
  int scoreCall = 0;
  for (const auto& locType : locTypes)
  {
    if (locType.c_str()[0] == 'C')
    {
      QueryGetMktCtyZoneHistorical SQLMktCtyZone(_dbAdapt);
      scoreCall =
          SQLMktCtyZone.getScore(zoneTypeStr, inclExcl, loc, zoneStr, vendor, startDate, endDate);
    }
    else if (locType.c_str()[0] == 'S')
    {
      QueryGetMktStateZoneHistorical SQLMktStateZone(_dbAdapt);
      scoreCall =
          SQLMktStateZone.getScore(zoneTypeStr, inclExcl, loc, zoneStr, vendor, startDate, endDate);
    }
    else if (locType.c_str()[0] == 'N')
    {
      QueryGetMktNationZoneHistorical SQLMktNationZone(_dbAdapt);
      scoreCall = SQLMktNationZone.getScore(
          zoneTypeStr, inclExcl, loc, zoneStr, vendor, startDate, endDate);
    }
    else if (locType.c_str()[0] == '*')
    {
      QueryGetMktSubAreaZoneHistorical SQLMktSubAreaZone(_dbAdapt);
      scoreCall = SQLMktSubAreaZone.getScore(
          zoneTypeStr, inclExcl, loc, zoneStr, vendor, startDate, endDate);
    }
    else if (locType.c_str()[0] == 'A')
    {
      QueryGetMktAreaZoneHistorical SQLMktAreaZone(_dbAdapt);
      scoreCall =
          SQLMktAreaZone.getScore(zoneTypeStr, inclExcl, loc, zoneStr, vendor, startDate, endDate);
    }
    else if (locType.c_str()[0] == 'Z')
    {
      QueryGetMktZoneZoneHistorical SQLMktZoneZone(_dbAdapt);
      zoneZone =
          SQLMktZoneZone.getZones(zoneTypeStr, inclExcl, zoneStr, vendor, startDate, endDate);

      if (zoneType == 'U' || zoneType == 'T')
        zoneType = 'R'; // User defined zones are made up of 'R' type zones

      size_t j;
      for (j = 0; j < zoneZone.size(); j++)
      {
        if ((scoreCall = isLocInZone(loc, zoneZone[j], zoneType, vendor, startDate, endDate)))
        {
          break;
        }
      } // for
    } // Zone

    score = score < scoreCall ? scoreCall : score;
  } // for()
  return score;
} // QueryGetSITAAddonScoreHistorical::evaluateZone()

///////////////////////////////////////////////////////////////////////
//  QueryGetZoneLocTypes
///////////////////////////////////////////////////////////////////////
const char*
QueryGetZoneLocTypes::getQueryName() const
{
  return "GETZONELOCTYPES";
}

void
QueryGetZoneLocTypes::initialize()
{
  if (!_isInitialized)
  {
    QueryGetZoneLocTypesSQLStatement<QueryGetZoneLocTypes> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETZONELOCTYPES");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

std::vector<std::string>
QueryGetZoneLocTypes::getLocTypes(char* zone,
                                  char* zoneType,
                                  std::string inclExcl,
                                  const VendorCode& vendor)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  std::string saveOrgSQL = _baseSQL;

  resetSQL();

  substParm(zone, 1);
  substParm(zoneType, 2);
  substParm(inclExcl, 3);
  substParm(vendor, 4);
  substCurrentDate();
  std::vector<std::string> ret;
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  if (!(row = res.nextRow()))
  { // Nothin found
    if (isReserveZone(zoneType))
    { // Use only ATP for Reserve Zones!
      res.freeResult();
      _baseSQL = saveOrgSQL;
      resetSQL();
      substParm(zone, 1);
      substParm(zoneType, 2);
      substParm(inclExcl, 3);
      substParm("ATP", 4);
      substCurrentDate();
      res.executeQuery(this);
    } // if (zoneType R or T)
  } // if (Nothin Found in original query)
  else // 1st shot was good
    ret.push_back(QueryGetZoneLocTypesSQLStatement<QueryGetZoneLocTypes>::mapRowToString(row));
  while ((row = res.nextRow()))
  { // Get the rest of them from whichever query got sumpin
    ret.push_back(QueryGetZoneLocTypesSQLStatement<QueryGetZoneLocTypes>::mapRowToString(row));
  }
  LOG4CXX_INFO(_logger,
               "GETZONELOCTYPES: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                             << stopCPU() << ")");
  res.freeResult();
  return ret;
} // QueryGetZoneLocTypes::getLocTypes()

///////////////////////////////////////////////////////////////////////
//  QueryGetZoneLocTypesHistorical
///////////////////////////////////////////////////////////////////////
const char*
QueryGetZoneLocTypesHistorical::getQueryName() const
{
  return "GETZONELOCTYPESHISTORICAL";
}

void
QueryGetZoneLocTypesHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetZoneLocTypesHistoricalSQLStatement<QueryGetZoneLocTypesHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETZONELOCTYPESHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

std::vector<std::string>
QueryGetZoneLocTypesHistorical::getLocTypes(char* zone,
                                            char* zoneType,
                                            std::string inclExcl,
                                            const VendorCode& vendor,
                                            const DateTime& startDate,
                                            const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  std::string saveOrgSQL = _baseSQL;

  resetSQL();

  substParm(1, zone);
  substParm(2, zoneType);
  substParm(3, inclExcl);
  substParm(4, vendor);
  substParm(5, startDate);
  substParm(6, endDate);
  substParm(7, endDate);
  substParm(8, zone);
  substParm(9, zoneType);
  substParm(10, inclExcl);
  substParm(11, vendor);
  substParm(12, startDate);
  substParm(13, endDate);
  substParm(14, endDate);
  std::vector<std::string> ret;
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  if (!(row = res.nextRow()))
  { // Nothin found
    if(isReserveZone(zoneType))
    { // Use only ATP for Reserve Zones!
      res.freeResult();
      _baseSQL = saveOrgSQL;
      resetSQL();
      substParm(1, zone);
      substParm(2, zoneType);
      substParm(3, inclExcl);
      substParm(4, "ATP");
      substParm(5, startDate);
      substParm(6, endDate);
      substParm(7, endDate);
      substParm(8, zone);
      substParm(9, zoneType);
      substParm(10, inclExcl);
      substParm(11, "ATP");
      substParm(12, startDate);
      substParm(13, endDate);
      substParm(14, endDate);
      res.executeQuery(this);
    } // if (zoneType R or T)
  } // if (Nothin Found in original query)
  else // 1st shot was good
    ret.push_back(
        QueryGetZoneLocTypesHistoricalSQLStatement<QueryGetZoneLocTypesHistorical>::mapRowToString(
            row));
  while ((row = res.nextRow()))
  { // Get the rest of them from whichever query got sumpin
    ret.push_back(
        QueryGetZoneLocTypesHistoricalSQLStatement<QueryGetZoneLocTypesHistorical>::mapRowToString(
            row));
  }
  LOG4CXX_INFO(_logger,
               "GETZONELOCTYPESHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                       << " (" << stopCPU() << ")");
  res.freeResult();
  return ret;
} // QueryGetZoneLocTypesHistorical::getLocTypes()

///////////////////////////////////////////////////////////////////////
//  QueryGetMktCtyZone
///////////////////////////////////////////////////////////////////////
const char*
QueryGetMktCtyZone::getQueryName() const
{
  return "GETMKTCTYZONE";
}

void
QueryGetMktCtyZone::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMktCtyZoneSQLStatement<QueryGetMktCtyZone> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMKTCTYZONE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

int
QueryGetMktCtyZone::getScore(char* zoneType,
                             std::string inclExcl,
                             const tse::LocCode& loc,
                             char* zone,
                             const VendorCode& vendor)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  std::string saveOrgSQL = _baseSQL;

  resetSQL();

  substParm(zoneType, 1);
  substParm(inclExcl, 2);
  substParm(loc, 3);
  substParm(zone, 4);
  substParm(zone, 5);
  substParm(vendor, 6);
  substCurrentDate();
  int ret = 0;
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  if (!(row = res.nextRow()))
  {
    if (isReserveZone(zoneType))
    { // Use ATP for Reserve Zones only!
      res.freeResult();
      _baseSQL = saveOrgSQL;
      resetSQL();
      substParm(zoneType, 1);
      substParm(inclExcl, 2);
      substParm(loc, 3);
      substParm(zone, 4);
      substParm(zone, 5);
      substParm("ATP", 6);
      substCurrentDate();
      if ((row = res.nextRow()))
      {
        ret = QueryGetMktCtyZoneSQLStatement<QueryGetMktCtyZone>::mapRowToInt(row);
      }
    } // if (ATP retry for Reserve Zone)
  } // if (Nothin found)
  else // 1st shot was good
  {
    ret = QueryGetMktCtyZoneSQLStatement<QueryGetMktCtyZone>::mapRowToInt(row);
  } // else
  LOG4CXX_INFO(_logger,
               "GETMKTCTYZONE: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                           << stopCPU() << ")");
  res.freeResult();
  return ret;
} // QueryGetMktCtyZone::getScore()

///////////////////////////////////////////////////////////////////////
//  QueryGetMktCtyZoneHistorical
///////////////////////////////////////////////////////////////////////
const char*
QueryGetMktCtyZoneHistorical::getQueryName() const
{
  return "GETMKTCTYZONEHISTORICAL";
}

void
QueryGetMktCtyZoneHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMktCtyZoneHistoricalSQLStatement<QueryGetMktCtyZoneHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMKTCTYZONEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

int
QueryGetMktCtyZoneHistorical::getScore(char* zoneType,
                                       std::string inclExcl,
                                       const tse::LocCode& loc,
                                       char* zone,
                                       const VendorCode& vendor,
                                       const DateTime& startDate,
                                       const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  std::string saveOrgSQL = _baseSQL;

  resetSQL();

  substParm(1, zoneType);
  substParm(2, inclExcl);
  substParm(3, loc);
  substParm(4, zone);
  substParm(5, zone);
  substParm(6, vendor);
  substParm(7, startDate);
  substParm(8, endDate);
  substParm(9, endDate);
  substParm(10, zoneType);
  substParm(11, inclExcl);
  substParm(12, loc);
  substParm(13, zone);
  substParm(14, zone);
  substParm(15, vendor);
  substParm(16, startDate);
  substParm(17, endDate);
  substParm(18, endDate);
  int ret = 0;
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  if (!(row = res.nextRow()))
  {
    if (isReserveZone(zoneType))
    { // Use ATP for Reserve Zones only!
      res.freeResult();
      _baseSQL = saveOrgSQL;
      resetSQL();
      substParm(1, zoneType);
      substParm(2, inclExcl);
      substParm(3, loc);
      substParm(4, zone);
      substParm(5, zone);
      substParm(6, "ATP");
      substParm(7, startDate);
      substParm(8, endDate);
      substParm(9, endDate);
      substParm(10, zoneType);
      substParm(11, inclExcl);
      substParm(12, loc);
      substParm(13, zone);
      substParm(14, zone);
      substParm(15, "ATP");
      substParm(16, startDate);
      substParm(17, endDate);
      substParm(18, endDate);
      res.executeQuery(this);
      if ((row = res.nextRow()))
      {
        ret = QueryGetMktCtyZoneHistoricalSQLStatement<QueryGetMktCtyZoneHistorical>::mapRowToInt(
            row);
      }
    } // if (ATP retry for Reserve Zone)
  } // if (Nothin found)
  else // 1st shot was good
  {
    ret = QueryGetMktCtyZoneHistoricalSQLStatement<QueryGetMktCtyZoneHistorical>::mapRowToInt(row);
  } // else
  LOG4CXX_INFO(_logger,
               "GETMKTCTYZONEHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                     << " (" << stopCPU() << ")");
  res.freeResult();
  return ret;
} // QueryGetMktCtyZoneHistorical::getScore()

///////////////////////////////////////////////////////////////////////
//  QueryGetMktStateZone
///////////////////////////////////////////////////////////////////////
const char*
QueryGetMktStateZone::getQueryName() const
{
  return "GETMKTSTATEZONE";
}

void
QueryGetMktStateZone::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMktStateZoneSQLStatement<QueryGetMktStateZone> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMKTSTATEZONE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

int
QueryGetMktStateZone::getScore(char* zoneType,
                               std::string inclExcl,
                               const tse::LocCode& loc,
                               char* zone,
                               const VendorCode& vendor)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  std::string saveOrgSQL = _baseSQL;

  resetSQL();

  substParm(zoneType, 1);
  substParm(inclExcl, 2);
  substParm(loc, 3);
  substParm(zone, 4);
  substParm(zone, 5);
  substParm(vendor, 6);
  substCurrentDate();
  int ret = 0;
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  if (!(row = res.nextRow()))
  {
    if (isReserveZone(zoneType))
    { // Use ATP for Reserve Zones only!
      res.freeResult();
      _baseSQL = saveOrgSQL;
      resetSQL();
      substParm(zoneType, 1);
      substParm(inclExcl, 2);
      substParm(loc, 3);
      substParm(zone, 4);
      substParm(zone, 5);
      substParm("ATP", 6);
      substCurrentDate();
      res.executeQuery(this);
      if ((row = res.nextRow()))
      {
        ret = QueryGetMktStateZoneSQLStatement<QueryGetMktStateZone>::mapRowToInt(row);
      }
    } // if (ATP retry for Reserve Zone)
  } // if (Nothin found)
  else // 1st shot was good
  {
    ret = QueryGetMktStateZoneSQLStatement<QueryGetMktStateZone>::mapRowToInt(row);
  } // else
  LOG4CXX_INFO(_logger,
               "GETMKTSTATEZONE: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                             << stopCPU() << ")");
  res.freeResult();
  return ret;
} // QueryGetMktStateZone::getScore()

///////////////////////////////////////////////////////////////////////
//  QueryGetMktStateZoneHistorical
///////////////////////////////////////////////////////////////////////
const char*
QueryGetMktStateZoneHistorical::getQueryName() const
{
  return "GETMKTSTATEZONEHISTORICAL";
}

void
QueryGetMktStateZoneHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMktStateZoneHistoricalSQLStatement<QueryGetMktStateZoneHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMKTSTATEZONEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

int
QueryGetMktStateZoneHistorical::getScore(char* zoneType,
                                         std::string inclExcl,
                                         const tse::LocCode& loc,
                                         char* zone,
                                         const VendorCode& vendor,
                                         const DateTime& startDate,
                                         const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  std::string saveOrgSQL = _baseSQL;

  resetSQL();

  substParm(1, zoneType);
  substParm(2, inclExcl);
  substParm(3, loc);
  substParm(4, zone);
  substParm(5, zone);
  substParm(6, vendor);
  substParm(7, startDate);
  substParm(8, endDate);
  substParm(9, endDate);
  substParm(10, startDate);
  substParm(11, endDate);
  substParm(12, endDate);
  substParm(13, zoneType);
  substParm(14, inclExcl);
  substParm(15, loc);
  substParm(16, zone);
  substParm(17, zone);
  substParm(18, vendor);
  substParm(19, startDate);
  substParm(20, endDate);
  substParm(21, endDate);
  substParm(22, startDate);
  substParm(23, endDate);
  substParm(24, endDate);
  int ret = 0;
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  if (!(row = res.nextRow()))
  {
    if(isReserveZone(zoneType))
    { // Use ATP for Reserve Zones only!
      res.freeResult();
      _baseSQL = saveOrgSQL;
      resetSQL();
      substParm(1, zoneType);
      substParm(2, inclExcl);
      substParm(3, loc);
      substParm(4, zone);
      substParm(5, zone);
      substParm(6, "ATP");
      substParm(7, startDate);
      substParm(8, endDate);
      substParm(9, endDate);
      substParm(10, startDate);
      substParm(11, endDate);
      substParm(12, endDate);
      substParm(13, zoneType);
      substParm(14, inclExcl);
      substParm(15, loc);
      substParm(16, zone);
      substParm(17, zone);
      substParm(18, "ATP");
      substParm(19, startDate);
      substParm(20, endDate);
      substParm(21, endDate);
      substParm(22, startDate);
      substParm(23, endDate);
      substParm(24, endDate);
      res.executeQuery(this);
      if ((row = res.nextRow()))
      {
        ret =
            QueryGetMktStateZoneHistoricalSQLStatement<QueryGetMktStateZoneHistorical>::mapRowToInt(
                row);
      }
    } // if (ATP retry for Reserve Zone)
  } // if (Nothin found)
  else // 1st shot was good
  {
    ret = QueryGetMktStateZoneHistoricalSQLStatement<QueryGetMktStateZoneHistorical>::mapRowToInt(
        row);
  } // else
  LOG4CXX_INFO(_logger,
               "GETMKTSTATEZONEHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                       << " (" << stopCPU() << ")");
  res.freeResult();
  return ret;
} // QueryGetMktStateZoneHistorical::getScore()

///////////////////////////////////////////////////////////////////////
//  QueryGetMktNationZone
///////////////////////////////////////////////////////////////////////
const char*
QueryGetMktNationZone::getQueryName() const
{
  return "GETMKTNATIONZONE";
}

void
QueryGetMktNationZone::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMktNationZoneSQLStatement<QueryGetMktNationZone> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMKTNATIONZONE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

int
QueryGetMktNationZone::getScore(char* zoneType,
                                std::string inclExcl,
                                const tse::LocCode& loc,
                                char* zone,
                                const VendorCode& vendor)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  std::string saveOrgSQL = _baseSQL;

  resetSQL();

  substParm(zoneType, 1);
  substParm(inclExcl, 2);
  substParm(loc, 3);
  substParm(zone, 4);
  substParm(zone, 5);
  substParm(vendor, 6);
  substCurrentDate();
  int ret = 0;
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  if (!(row = res.nextRow()))
  {
    if(isReserveZone(zoneType))
    { // Use ATP for Reserve Zones only!
      res.freeResult();
      _baseSQL = saveOrgSQL;
      resetSQL();
      substParm(zoneType, 1);
      substParm(inclExcl, 2);
      substParm(loc, 3);
      substParm(zone, 4);
      substParm(zone, 5);
      substParm("ATP", 6);
      substCurrentDate();
      res.executeQuery(this);
      if ((row = res.nextRow()))
      {
        ret = QueryGetMktNationZoneSQLStatement<QueryGetMktNationZone>::mapRowToInt(row);
      }
    } // if (ATP retry for Reserve Zone)
  } // if (Nothin found)
  else // 1st shot was good
  {
    ret = QueryGetMktNationZoneSQLStatement<QueryGetMktNationZone>::mapRowToInt(row);
  } // else
  LOG4CXX_INFO(_logger,
               "GETMKTNATIONZONE: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                              << stopCPU() << ")");
  res.freeResult();
  return ret;
} // QueryGetMktNationZone::getScore()

///////////////////////////////////////////////////////////////////////
//  QueryGetMktNationZoneHistorical
///////////////////////////////////////////////////////////////////////
const char*
QueryGetMktNationZoneHistorical::getQueryName() const
{
  return "GETMKTNATIONZONEHISTORICAL";
}

void
QueryGetMktNationZoneHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMktNationZoneHistoricalSQLStatement<QueryGetMktNationZoneHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMKTNATIONZONEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

int
QueryGetMktNationZoneHistorical::getScore(char* zoneType,
                                          std::string inclExcl,
                                          const tse::LocCode& loc,
                                          char* zone,
                                          const VendorCode& vendor,
                                          const DateTime& startDate,
                                          const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  std::string saveOrgSQL = _baseSQL;

  resetSQL();

  substParm(1, zoneType);
  substParm(2, inclExcl);
  substParm(3, loc);
  substParm(4, zone);
  substParm(5, zone);
  substParm(6, vendor);
  substParm(7, startDate);
  substParm(8, endDate);
  substParm(9, endDate);
  substParm(10, startDate);
  substParm(11, endDate);
  substParm(12, endDate);
  substParm(13, zoneType);
  substParm(14, inclExcl);
  substParm(15, loc);
  substParm(16, zone);
  substParm(17, zone);
  substParm(18, vendor);
  substParm(19, startDate);
  substParm(20, endDate);
  substParm(21, endDate);
  substParm(22, startDate);
  substParm(23, endDate);
  substParm(24, endDate);
  int ret = 0;
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  if (!(row = res.nextRow()))
  {
    if(isReserveZone(zoneType))
    { // Use ATP for Reserve Zones only!
      res.freeResult();
      _baseSQL = saveOrgSQL;
      resetSQL();
      substParm(1, zoneType);
      substParm(2, inclExcl);
      substParm(3, loc);
      substParm(4, zone);
      substParm(5, zone);
      substParm(6, "ATP");
      substParm(7, startDate);
      substParm(8, endDate);
      substParm(9, endDate);
      substParm(10, startDate);
      substParm(11, endDate);
      substParm(12, endDate);
      substParm(13, zoneType);
      substParm(14, inclExcl);
      substParm(15, loc);
      substParm(16, zone);
      substParm(17, zone);
      substParm(18, "ATP");
      substParm(19, startDate);
      substParm(20, endDate);
      substParm(21, endDate);
      substParm(22, startDate);
      substParm(23, endDate);
      substParm(24, endDate);
      res.executeQuery(this);
      if ((row = res.nextRow()))
      {
        ret = QueryGetMktNationZoneHistoricalSQLStatement<
            QueryGetMktNationZoneHistorical>::mapRowToInt(row);
      }
    } // if (ATP retry for Reserve Zone)
  } // if (Nothin found)
  else // 1st shot was good
  {
    ret = QueryGetMktNationZoneHistoricalSQLStatement<QueryGetMktNationZoneHistorical>::mapRowToInt(
        row);
  } // else
  LOG4CXX_INFO(_logger,
               "GETMKTNATIONZONEHISTORICAL: NumRows = " << res.numRows() << " Time = "
                                                        << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
  return ret;
} // QueryGetMktNationZoneHistorical::getScore()

///////////////////////////////////////////////////////////////////////
//  QueryGetMktSubAreaZone
///////////////////////////////////////////////////////////////////////
const char*
QueryGetMktSubAreaZone::getQueryName() const
{
  return "GETMKTSUBAREAZONE";
}

void
QueryGetMktSubAreaZone::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMktSubAreaZoneSQLStatement<QueryGetMktSubAreaZone> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMKTSUBAREAZONE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

int
QueryGetMktSubAreaZone::getScore(char* zoneType,
                                 std::string inclExcl,
                                 const tse::LocCode& loc,
                                 char* zone,
                                 const VendorCode& vendor)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  std::string saveOrgSQL = _baseSQL;

  resetSQL();

  substParm(zoneType, 1);
  substParm(inclExcl, 2);
  substParm(loc, 3);
  substParm(zone, 4);
  substParm(zone, 5);
  substParm(vendor, 6);
  substCurrentDate();
  int ret = 0;
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  if (!(row = res.nextRow()))
  {
    if(isReserveZone(zoneType))
    { // Use ATP for Reserve Zones only!
      res.freeResult();
      _baseSQL = saveOrgSQL;
      resetSQL();
      substParm(zoneType, 1);
      substParm(inclExcl, 2);
      substParm(loc, 3);
      substParm(zone, 4);
      substParm(zone, 5);
      substParm("ATP", 6);
      res.executeQuery(this);
      if ((row = res.nextRow()))
      {
        ret = QueryGetMktSubAreaZoneSQLStatement<QueryGetMktSubAreaZone>::mapRowToInt(row);
      }
    } // if (ATP retry for Reserve Zone)
  } // if (Nothin found)
  else // 1st shot was good
  {
    ret = QueryGetMktSubAreaZoneSQLStatement<QueryGetMktSubAreaZone>::mapRowToInt(row);
  } // else
  LOG4CXX_INFO(_logger,
               "GETMKTSUBAREAZONE: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                               << stopCPU() << ")");
  res.freeResult();
  return ret;
} // QueryGetMktSubAreaZone::getScore()

///////////////////////////////////////////////////////////////////////
//  QueryGetMktSubAreaZoneHistorical
///////////////////////////////////////////////////////////////////////
const char*
QueryGetMktSubAreaZoneHistorical::getQueryName() const
{
  return "GETMKTSUBAREAZONEHISTORICAL";
}

void
QueryGetMktSubAreaZoneHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMktSubAreaZoneHistoricalSQLStatement<QueryGetMktSubAreaZoneHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMKTSUBAREAZONEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

int
QueryGetMktSubAreaZoneHistorical::getScore(char* zoneType,
                                           std::string inclExcl,
                                           const tse::LocCode& loc,
                                           char* zone,
                                           const VendorCode& vendor,
                                           const DateTime& startDate,
                                           const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  std::string saveOrgSQL = _baseSQL;

  resetSQL();

  substParm(1, zoneType);
  substParm(2, inclExcl);
  substParm(3, loc);
  substParm(4, zone);
  substParm(5, zone);
  substParm(6, vendor);
  substParm(7, startDate);
  substParm(8, endDate);
  substParm(9, endDate);
  substParm(10, startDate);
  substParm(11, endDate);
  substParm(12, endDate);
  substParm(13, zoneType);
  substParm(14, inclExcl);
  substParm(15, loc);
  substParm(16, zone);
  substParm(17, zone);
  substParm(18, vendor);
  substParm(19, startDate);
  substParm(20, endDate);
  substParm(21, endDate);
  substParm(22, startDate);
  substParm(23, endDate);
  substParm(24, endDate);
  int ret = 0;
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  if (!(row = res.nextRow()))
  {
    if(isReserveZone(zoneType))
    { // Use ATP for Reserve Zones only!
      res.freeResult();
      _baseSQL = saveOrgSQL;
      resetSQL();
      substParm(1, zoneType);
      substParm(2, inclExcl);
      substParm(3, loc);
      substParm(4, zone);
      substParm(5, zone);
      substParm(6, "ATP");
      substParm(7, startDate);
      substParm(8, endDate);
      substParm(9, endDate);
      substParm(10, startDate);
      substParm(11, endDate);
      substParm(12, endDate);
      substParm(13, zoneType);
      substParm(14, inclExcl);
      substParm(15, loc);
      substParm(16, zone);
      substParm(17, zone);
      substParm(18, "ATP");
      substParm(19, startDate);
      substParm(20, endDate);
      substParm(21, endDate);
      substParm(22, startDate);
      substParm(23, endDate);
      substParm(24, endDate);
      res.executeQuery(this);
      if ((row = res.nextRow()))
      {
        ret = QueryGetMktSubAreaZoneHistoricalSQLStatement<
            QueryGetMktSubAreaZoneHistorical>::mapRowToInt(row);
      }
    } // if (ATP retry for Reserve Zone)
  } // if (Nothin found)
  else // 1st shot was good
  {
    ret =
        QueryGetMktSubAreaZoneHistoricalSQLStatement<QueryGetMktSubAreaZoneHistorical>::mapRowToInt(
            row);
  } // else
  LOG4CXX_INFO(_logger,
               "GETMKTSUBAREAZONEHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
  return ret;
} // QueryGetMktSubAreaZoneHistorical::getScore()

///////////////////////////////////////////////////////////////////////
//  QueryGetMktAreaZone
///////////////////////////////////////////////////////////////////////
const char*
QueryGetMktAreaZone::getQueryName() const
{
  return "GETMKTAREAZONE";
}

void
QueryGetMktAreaZone::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMktAreaZoneSQLStatement<QueryGetMktAreaZone> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMKTAREAZONE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

int
QueryGetMktAreaZone::getScore(char* zoneType,
                              std::string inclExcl,
                              const tse::LocCode& loc,
                              char* zone,
                              const VendorCode& vendor)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  std::string saveOrgSQL = _baseSQL;

  resetSQL();

  substParm(zoneType, 1);
  substParm(inclExcl, 2);
  substParm(loc, 3);
  substParm(zone, 4);
  substParm(zone, 5);
  substParm(vendor, 6);
  substCurrentDate();
  int ret = 0;
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  if (!(row = res.nextRow()))
  {
    if(isReserveZone(zoneType))
    { // Use ATP for Reserve Zones only!
      res.freeResult();
      _baseSQL = saveOrgSQL;
      resetSQL();
      substParm(zoneType, 1);
      substParm(inclExcl, 2);
      substParm(loc, 3);
      substParm(zone, 4);
      substParm(zone, 5);
      substParm("ATP", 6);
      substCurrentDate();
      res.executeQuery(this);
      if ((row = res.nextRow()))
      {
        ret = QueryGetMktAreaZoneSQLStatement<QueryGetMktAreaZone>::mapRowToInt(row);
      }
    } // if (ATP retry for Reserve Zone)
  } // if (Nothin found)
  else // 1st shot was good
  {
    ret = QueryGetMktAreaZoneSQLStatement<QueryGetMktAreaZone>::mapRowToInt(row);
  } // else
  LOG4CXX_INFO(_logger,
               "GETMKTAREAZONE: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                            << stopCPU() << ")");
  res.freeResult();
  return ret;
} // QueryGetMktAreaZone::getScore()

///////////////////////////////////////////////////////////////////////
//  QueryGetMktAreaZoneHistorical
///////////////////////////////////////////////////////////////////////
const char*
QueryGetMktAreaZoneHistorical::getQueryName() const
{
  return "GETMKTAREAZONEHISTORICAL";
}

void
QueryGetMktAreaZoneHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMktAreaZoneHistoricalSQLStatement<QueryGetMktAreaZoneHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMKTAREAZONEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

int
QueryGetMktAreaZoneHistorical::getScore(char* zoneType,
                                        std::string inclExcl,
                                        const tse::LocCode& loc,
                                        char* zone,
                                        const VendorCode& vendor,
                                        const DateTime& startDate,
                                        const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  std::string saveOrgSQL = _baseSQL;

  resetSQL();

  substParm(1, zoneType);
  substParm(2, inclExcl);
  substParm(3, loc);
  substParm(4, zone);
  substParm(5, zone);
  substParm(6, vendor);
  substParm(7, startDate);
  substParm(8, endDate);
  substParm(9, endDate);
  substParm(10, startDate);
  substParm(11, endDate);
  substParm(12, endDate);
  substParm(13, zoneType);
  substParm(14, inclExcl);
  substParm(15, loc);
  substParm(16, zone);
  substParm(17, zone);
  substParm(18, vendor);
  substParm(19, startDate);
  substParm(20, endDate);
  substParm(21, endDate);
  substParm(22, startDate);
  substParm(23, endDate);
  substParm(24, endDate);
  int ret = 0;
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  if (!(row = res.nextRow()))
  {
    if(isReserveZone(zoneType))
    { // Use ATP for Reserve Zones only!
      res.freeResult();
      _baseSQL = saveOrgSQL;
      resetSQL();
      substParm(1, zoneType);
      substParm(2, inclExcl);
      substParm(3, loc);
      substParm(4, zone);
      substParm(5, zone);
      substParm(6, "ATP");
      substParm(7, startDate);
      substParm(8, endDate);
      substParm(9, endDate);
      substParm(10, startDate);
      substParm(11, endDate);
      substParm(12, endDate);
      substParm(13, zoneType);
      substParm(14, inclExcl);
      substParm(15, loc);
      substParm(16, zone);
      substParm(17, zone);
      substParm(18, "ATP");
      substParm(19, startDate);
      substParm(20, endDate);
      substParm(21, endDate);
      substParm(22, startDate);
      substParm(23, endDate);
      substParm(24, endDate);
      res.executeQuery(this);
      if ((row = res.nextRow()))
      {
        ret = QueryGetMktAreaZoneHistoricalSQLStatement<QueryGetMktAreaZoneHistorical>::mapRowToInt(
            row);
      }
    } // if (ATP retry for Reserve Zone)
  } // if (Nothin found)
  else // 1st shot was good
  {
    ret =
        QueryGetMktAreaZoneHistoricalSQLStatement<QueryGetMktAreaZoneHistorical>::mapRowToInt(row);
  } // else
  LOG4CXX_INFO(_logger,
               "GETMKTAREAZONEHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ")");
  res.freeResult();
  return ret;
} // QueryGetMktAreaZoneHistorical::getScore()

///////////////////////////////////////////////////////////////////////
//  QueryGetMktZoneZone
///////////////////////////////////////////////////////////////////////
const char*
QueryGetMktZoneZone::getQueryName() const
{
  return "GETMKTZONEZONE";
}

void
QueryGetMktZoneZone::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMktZoneZoneSQLStatement<QueryGetMktZoneZone> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMKTZONEZONE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

std::vector<int>
QueryGetMktZoneZone::getZones(char* zoneType,
                              std::string inclExcl,
                              char* zone,
                              const VendorCode& vendor)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  std::string saveOrgSQL = _baseSQL;

  resetSQL();

  substParm(zoneType, 1);
  substParm(inclExcl, 2);
  substParm(zone, 3);
  substParm(zone, 4);
  substParm(vendor, 5);
  substCurrentDate();
  std::vector<int> ret;
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  if (!(row = res.nextRow()))
  {
    if(isReserveZone(zoneType))
    { // Use ATP for Reserve Zones only!
      res.freeResult();
      _baseSQL = saveOrgSQL;
      resetSQL();
      substParm(zoneType, 1);
      substParm(inclExcl, 2);
      substParm(zone, 3);
      substParm(zone, 4);
      substParm("ATP", 5);
      res.executeQuery(this);
    } // if (ATP retry for Reserve Zone)
  } // if (Nothin found)
  else // 1st shot was good
    ret.push_back(QueryGetMktZoneZoneSQLStatement<QueryGetMktZoneZone>::mapRowToInt(row));
  while ((row = res.nextRow()))
  {
    ret.push_back(QueryGetMktZoneZoneSQLStatement<QueryGetMktZoneZone>::mapRowToInt(row));
  }
  LOG4CXX_INFO(_logger,
               "GETMKTZONEZONE: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                            << stopCPU() << ") mSecs");
  res.freeResult();
  return ret;
} // QueryGetMktZoneZone::getZones()

///////////////////////////////////////////////////////////////////////
//  QueryGetMktZoneZoneHistorical
///////////////////////////////////////////////////////////////////////
const char*
QueryGetMktZoneZoneHistorical::getQueryName() const
{
  return "GETMKTZONEZONEHISTORICAL";
}

void
QueryGetMktZoneZoneHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMktZoneZoneHistoricalSQLStatement<QueryGetMktZoneZoneHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMKTZONEZONEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

std::vector<int>
QueryGetMktZoneZoneHistorical::getZones(char* zoneType,
                                        std::string inclExcl,
                                        char* zone,
                                        const VendorCode& vendor,
                                        const DateTime& startDate,
                                        const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  std::string saveOrgSQL = _baseSQL;

  resetSQL();

  substParm(1, zoneType);
  substParm(2, inclExcl);
  substParm(3, zone);
  substParm(4, zone);
  substParm(5, vendor);
  substParm(6, startDate);
  substParm(7, endDate);
  substParm(8, endDate);
  substParm(9, zoneType);
  substParm(10, inclExcl);
  substParm(11, zone);
  substParm(12, zone);
  substParm(13, vendor);
  substParm(14, startDate);
  substParm(15, endDate);
  substParm(16, endDate);
  std::vector<int> ret;
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  if (!(row = res.nextRow()))
  {
    if(isReserveZone(zoneType))
    { // Use ATP for Reserve Zones only!
      res.freeResult();
      _baseSQL = saveOrgSQL;
      resetSQL();
      substParm(1, zoneType);
      substParm(2, inclExcl);
      substParm(3, zone);
      substParm(4, zone);
      substParm(5, "ATP");
      substParm(6, startDate);
      substParm(7, endDate);
      substParm(8, endDate);
      substParm(9, zoneType);
      substParm(10, inclExcl);
      substParm(11, zone);
      substParm(12, zone);
      substParm(13, "ATP");
      substParm(14, startDate);
      substParm(15, endDate);
      substParm(16, endDate);
      res.executeQuery(this);
    } // if (ATP retry for Reserve Zone)
  } // if (Nothin found)
  else // 1st shot was good
    ret.push_back(
        QueryGetMktZoneZoneHistoricalSQLStatement<QueryGetMktZoneZoneHistorical>::mapRowToInt(row));
  while ((row = res.nextRow()))
  {
    ret.push_back(
        QueryGetMktZoneZoneHistoricalSQLStatement<QueryGetMktZoneZoneHistorical>::mapRowToInt(row));
  }
  LOG4CXX_INFO(_logger,
               "GETMKTZONEZONEHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ") mSecs");
  res.freeResult();
  return ret;
} // QueryGetMktZoneZoneHistorical::getZones()
} // tse
