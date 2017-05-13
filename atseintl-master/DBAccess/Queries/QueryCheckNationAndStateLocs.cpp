//----------------------------------------------------------------------------
//          File:           QueryCheckNationAndStateLocs.cpp
//          Description:    QueryCheckNationAndStateLocs
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
#include "DBAccess/Queries/QueryCheckNationAndStateLocs.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryCheckNationAndStateLocsSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryCheckNationInSubArea::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.CheckNationInSubArea"));
std::string QueryCheckNationInSubArea::_baseSQL;
bool QueryCheckNationInSubArea::_isInitialized = false;
SQLQueryInitializerHelper<QueryCheckNationInSubArea> g_CheckNationInSubArea;

const char*
QueryCheckNationInSubArea::getQueryName() const
{
  return "CHECKNATIONINSUBAREA";
};

void
QueryCheckNationInSubArea::initialize()
{
  if (!_isInitialized)
  {
    QueryCheckNationInSubAreaSQLStatement<QueryCheckNationInSubArea> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("CHECKNATIONINSUBAREA");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

bool
QueryCheckNationInSubArea::isNationInSubArea(const NationCode& nation, const LocCode& subArea)
{
  bool retVal = false;
  DBResultSet res(_dbAdapt);

  substParm(nation, 1);
  substParm(subArea, 2);
  substCurrentDate();
  res.executeQuery(this);
  if (res.nextRow())
    retVal = true;

  res.freeResult();
  return retVal;
} // QueryCheckNationInSubArea::isNationInSubArea()

///////////////////////////////////////////////////////////
//  QueryCheckNationInSubAreaHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryCheckNationInSubAreaHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.CheckNationInSubAreaHistorical"));
std::string QueryCheckNationInSubAreaHistorical::_baseSQL;
bool QueryCheckNationInSubAreaHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryCheckNationInSubAreaHistorical> g_CheckNationInSubAreaHistorical;

const char*
QueryCheckNationInSubAreaHistorical::getQueryName() const
{
  return "CHECKNATIONINSUBAREAHISTORICAL";
};

void
QueryCheckNationInSubAreaHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryCheckNationInSubAreaHistoricalSQLStatement<QueryCheckNationInSubAreaHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("CHECKNATIONINSUBAREAHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryCheckNationInSubAreaHistorical::findNationInSubArea(
    std::vector<tse::NationStateHistIsCurrChk*>& lstCC,
    const NationCode& nation,
    const LocCode& subArea,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, nation);
  substParm(2, subArea);
  substParm(3, startDate);
  substParm(4, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstCC.push_back(QueryCheckNationInSubAreaHistoricalSQLStatement<
        QueryCheckNationInSubAreaHistorical>::mapRowToNationStateHistIsCurrChk(row));
  }
  LOG4CXX_INFO(_logger,
               "CHECKNATIONINSUBAREAHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryCheckNationInSubAreaHistorical::findNationInSubArea()

///////////////////////////////////////////////////////////
//  QueryCheckNationInArea
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryCheckNationInArea::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.CheckNationInArea"));
std::string QueryCheckNationInArea::_baseSQL;
bool QueryCheckNationInArea::_isInitialized = false;
SQLQueryInitializerHelper<QueryCheckNationInArea> g_CheckNationInArea;

const char*
QueryCheckNationInArea::getQueryName() const
{
  return "CHECKNATIONINAREA";
};

void
QueryCheckNationInArea::initialize()
{
  if (!_isInitialized)
  {
    QueryCheckNationInAreaSQLStatement<QueryCheckNationInArea> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("CHECKNATIONINAREA");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

bool
QueryCheckNationInArea::isNationInArea(const NationCode& nation, const LocCode& area)
{
  bool retVal = false;
  DBResultSet res(_dbAdapt);

  substParm(nation, 1);
  substParm(area, 2);
  substCurrentDate();
  res.executeQuery(this);
  if (res.nextRow())
    retVal = true;

  res.freeResult();
  return retVal;
} // QueryCheckNationInArea::isNationInArea()

///////////////////////////////////////////////////////////
//  QueryCheckNationInAreaHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryCheckNationInAreaHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.CheckNationInAreaHistorical"));
std::string QueryCheckNationInAreaHistorical::_baseSQL;
bool QueryCheckNationInAreaHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryCheckNationInAreaHistorical> g_CheckNationInAreaHistorical;

const char*
QueryCheckNationInAreaHistorical::getQueryName() const
{
  return "CHECKNATIONINAREAHISTORICAL";
};

void
QueryCheckNationInAreaHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryCheckNationInAreaHistoricalSQLStatement<QueryCheckNationInAreaHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("CHECKNATIONINAREAHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryCheckNationInAreaHistorical::findNationInArea(
    std::vector<tse::NationStateHistIsCurrChk*>& lstCC,
    const NationCode& nation,
    const LocCode& area,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, nation);
  substParm(2, area);
  substParm(3, startDate);
  substParm(4, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstCC.push_back(QueryCheckNationInAreaHistoricalSQLStatement<
        QueryCheckNationInAreaHistorical>::mapRowToNationStateHistIsCurrChk(row));
  }

  LOG4CXX_INFO(_logger,
               "CHECKNATIONINAREAHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryCheckNationInAreaHistorical::findNationInArea()

///////////////////////////////////////////////////////////
//  QueryCheckNationInZone
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryCheckNationInZone::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.CheckNationInZone"));
std::string QueryCheckNationInZone::_baseSQL;
bool QueryCheckNationInZone::_isInitialized = false;
SQLQueryInitializerHelper<QueryCheckNationInZone> g_CheckNationInZone;

log4cxx::LoggerPtr
QueryCheckNationSubAreaInZone::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.CheckNationInZone.CheckNationSubAreaInZone"));
std::string QueryCheckNationSubAreaInZone::_baseSQL;
bool QueryCheckNationSubAreaInZone::_isInitialized = false;
SQLQueryInitializerHelper<QueryCheckNationSubAreaInZone> g_CheckNationSubAreaInZone;

log4cxx::LoggerPtr
QueryCheckNationAreaInZone::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.CheckNationInZone.CheckNationAreaInZone"));
std::string QueryCheckNationAreaInZone::_baseSQL;
bool QueryCheckNationAreaInZone::_isInitialized = false;
SQLQueryInitializerHelper<QueryCheckNationAreaInZone> g_CheckNationAreaInZone;

const char*
QueryCheckNationInZone::getQueryName() const
{
  return "CHECKNATIONINZONE";
};

void
QueryCheckNationInZone::initialize()
{
  if (!_isInitialized)
  {
    QueryCheckNationInZoneSQLStatement<QueryCheckNationInZone> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("CHECKNATIONINZONE");
    substTableDef(&_baseSQL);

    QueryCheckNationSubAreaInZone::initialize();
    QueryCheckNationAreaInZone::initialize();
    _isInitialized = true;
  }
} // initialize()

bool
QueryCheckNationInZone::isNationInZone(const VendorCode& vendor,
                                       int zoneNo,
                                       char zoneType,
                                       const NationCode& nation)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  std::string inclExcl = "";

  char znStr[15];
  sprintf(znStr, "%d", zoneNo);
  char zoneStr[] = "0000000";
  strcpy(&zoneStr[7 - strlen(znStr)], znStr);

  char zoneTypeStr[2];
  sprintf(zoneTypeStr, "%c", zoneType);

  resetSQL(); // For when called from isStateInZone()

  substParm(vendor, 1);
  substParm(zoneStr, 2);
  substParm(zoneTypeStr, 3);
  substParm(nation, 4);
  substCurrentDate();
  res.executeQuery(this);
  if ((row = res.nextRow()))
  {
    inclExcl = QueryCheckNationInZoneSQLStatement<QueryCheckNationInZone>::mapRowToString(row);
    res.freeResult();
    if (inclExcl == "I")
      return true;
    else
      return false;
  }
  res.freeResult();

  QueryCheckNationSubAreaInZone SQLNationSubAreaInZone(_dbAdapt);
  inclExcl = SQLNationSubAreaInZone.isNationSubAreaInZone(vendor, zoneStr, zoneTypeStr, nation);
  if (inclExcl == "I")
    return true;
  else if (inclExcl == "E")
    return false;

  QueryCheckNationAreaInZone SQLNationAreaInZone(_dbAdapt);
  inclExcl = SQLNationAreaInZone.isNationAreaInZone(vendor, zoneStr, zoneTypeStr, nation);
  if (inclExcl == "I")
    return true;

  return false;
} // QueryCheckNationInZone::isNationInZone()

///////////////////////////////////////////////////////////
//  QueryCheckNationInZoneHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryCheckNationInZoneHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.CheckNationInZoneHistorical"));
std::string QueryCheckNationInZoneHistorical::_baseSQL;
bool QueryCheckNationInZoneHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryCheckNationInZoneHistorical> g_CheckNationInZoneHistorical;

log4cxx::LoggerPtr
QueryCheckNationSubAreaInZoneHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.CheckNationInZoneHistorical.CheckNationSubAreaInZoneHistorical"));
std::string QueryCheckNationSubAreaInZoneHistorical::_baseSQL;
bool QueryCheckNationSubAreaInZoneHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryCheckNationSubAreaInZoneHistorical>
g_CheckNationSubAreaInZoneHistorical;

log4cxx::LoggerPtr
QueryCheckNationAreaInZoneHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.CheckNationInZoneHistorical.CheckNationAreaInZoneHistorical"));
std::string QueryCheckNationAreaInZoneHistorical::_baseSQL;
bool QueryCheckNationAreaInZoneHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryCheckNationAreaInZoneHistorical> g_CheckNationAreaInZoneHistorical;

const char*
QueryCheckNationInZoneHistorical::getQueryName() const
{
  return "CHECKNATIONINZONEHISTORICAL";
};

void
QueryCheckNationInZoneHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryCheckNationInZoneHistoricalSQLStatement<QueryCheckNationInZoneHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("CHECKNATIONINZONEHISTORICAL");
    substTableDef(&_baseSQL);

    QueryCheckNationSubAreaInZoneHistorical::initialize();
    QueryCheckNationAreaInZoneHistorical::initialize();
    _isInitialized = true;
  }
} // initialize()

void
QueryCheckNationInZoneHistorical::findNationInZone(
    std::vector<tse::NationStateHistIsCurrChk*>& lstCC,
    const VendorCode& vendor,
    int zoneNo,
    char zoneType,
    const NationCode& nation,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  char znStr[15];
  sprintf(znStr, "%d", zoneNo);
  char zoneStr[] = "0000000";
  strcpy(&zoneStr[7 - strlen(znStr)], znStr);

  char zoneTypeStr[2];
  sprintf(zoneTypeStr, "%c", zoneType);

  resetSQL(); // For when called from findStateInZone()

  substParm(1, vendor);
  substParm(2, zoneStr);
  substParm(3, zoneTypeStr);
  substParm(4, nation);
  substParm(5, startDate);
  substParm(6, endDate);
  substParm(7, endDate);
  substParm(8, vendor);
  substParm(9, zoneStr);
  substParm(10, zoneTypeStr);
  substParm(11, nation);
  substParm(12, startDate);
  substParm(13, endDate);
  substParm(14, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstCC.push_back(QueryCheckNationInZoneHistoricalSQLStatement<
        QueryCheckNationInZoneHistorical>::mapRowToNationStateHistIsCurrChk(row));
  }
  LOG4CXX_INFO(_logger,
               "CHECKNATIONINZONEHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();

  QueryCheckNationSubAreaInZoneHistorical SQLNationSubAreaInZone(_dbAdapt);
  SQLNationSubAreaInZone.findNationSubAreaInZone(
      lstCC, vendor, zoneStr, zoneTypeStr, nation, startDate, endDate);

  QueryCheckNationAreaInZoneHistorical SQLNationAreaInZone(_dbAdapt);
  SQLNationAreaInZone.findNationAreaInZone(
      lstCC, vendor, zoneStr, zoneTypeStr, nation, startDate, endDate);
} // QueryCheckNationInZoneHistorical::findNationInZone()

///////////////////////////////////////////////////////////
//  QueryCheckNationSubAreaInZone
///////////////////////////////////////////////////////////
const char*
QueryCheckNationSubAreaInZone::getQueryName() const
{
  return "CHECKNATIONSUBAREAINZONE";
};

void
QueryCheckNationSubAreaInZone::initialize()
{
  if (!_isInitialized)
  {
    QueryCheckNationSubAreaInZoneSQLStatement<QueryCheckNationSubAreaInZone> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("CHECKNATIONSUBAREAINZONE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

std::string
QueryCheckNationSubAreaInZone::isNationSubAreaInZone(const VendorCode& vendor,
                                                     char* zoneNo,
                                                     char* zoneType,
                                                     const NationCode& nation)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  std::string ret = "";

  resetSQL();

  substParm(vendor, 1);
  substParm(zoneNo, 2);
  substParm(zoneType, 3);
  substParm(nation, 4);
  substCurrentDate();
  res.executeQuery(this);
  if ((row = res.nextRow()))
  {
    ret = QueryCheckNationSubAreaInZoneSQLStatement<QueryCheckNationSubAreaInZone>::mapRowToString(
        row);
  }
  res.freeResult();
  return ret;
} // QueryCheckNationSubAreaInZone::isNationSubAreaInZone()

///////////////////////////////////////////////////////////
//  QueryCheckNationSubAreaInZoneHistorical
///////////////////////////////////////////////////////////
const char*
QueryCheckNationSubAreaInZoneHistorical::getQueryName() const
{
  return "CHECKNATIONSUBAREAINZONEHISTORICAL";
};

void
QueryCheckNationSubAreaInZoneHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryCheckNationSubAreaInZoneHistoricalSQLStatement<QueryCheckNationSubAreaInZoneHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("CHECKNATIONSUBAREAINZONEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryCheckNationSubAreaInZoneHistorical::findNationSubAreaInZone(
    std::vector<tse::NationStateHistIsCurrChk*>& lstCC,
    const VendorCode& vendor,
    char* zoneNo,
    char* zoneType,
    const NationCode& nation,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(1, vendor);
  substParm(2, zoneNo);
  substParm(3, zoneType);
  substParm(4, nation);
  substParm(5, startDate);
  substParm(6, endDate);
  substParm(7, endDate);
  substParm(8, startDate);
  substParm(9, endDate);
  substParm(10, vendor);
  substParm(11, zoneNo);
  substParm(12, zoneType);
  substParm(13, nation);
  substParm(14, startDate);
  substParm(15, endDate);
  substParm(16, endDate);
  substParm(17, startDate);
  substParm(18, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstCC.push_back(QueryCheckNationSubAreaInZoneHistoricalSQLStatement<
        QueryCheckNationSubAreaInZoneHistorical>::mapRowToNationStateHistIsCurrChk(row));
  }
  LOG4CXX_INFO(_logger,
               "CHECKNATIONSUBAREAINZONEHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryCheckNationSubAreaInZoneHistorical::findNationSubAreaInZone()

///////////////////////////////////////////////////////////
//  QueryCheckNationAreaInZone
///////////////////////////////////////////////////////////
const char*
QueryCheckNationAreaInZone::getQueryName() const
{
  return "CHECKNATIONAREAINZONE";
};

void
QueryCheckNationAreaInZone::initialize()
{
  if (!_isInitialized)
  {
    QueryCheckNationAreaInZoneSQLStatement<QueryCheckNationAreaInZone> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("CHECKNATIONAREAINZONE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

std::string
QueryCheckNationAreaInZone::isNationAreaInZone(const VendorCode& vendor,
                                               char* zoneNo,
                                               char* zoneType,
                                               const NationCode& nation)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  std::string ret = "";

  resetSQL();

  substParm(vendor, 1);
  substParm(zoneNo, 2);
  substParm(zoneType, 3);
  substParm(nation, 4);
  substCurrentDate();
  res.executeQuery(this);
  if ((row = res.nextRow()))
  {
    ret = QueryCheckNationAreaInZoneSQLStatement<QueryCheckNationAreaInZone>::mapRowToString(row);
  }
  res.freeResult();
  return ret;
} // QueryCheckNationAreaInZone::isNationAreaInZone()

///////////////////////////////////////////////////////////
//  QueryCheckNationAreaInZoneHistorical
///////////////////////////////////////////////////////////
const char*
QueryCheckNationAreaInZoneHistorical::getQueryName() const
{
  return "CHECKNATIONAREAINZONEHISTORICAL";
};

void
QueryCheckNationAreaInZoneHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryCheckNationAreaInZoneHistoricalSQLStatement<QueryCheckNationAreaInZoneHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("CHECKNATIONAREAINZONEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryCheckNationAreaInZoneHistorical::findNationAreaInZone(
    std::vector<tse::NationStateHistIsCurrChk*>& lstCC,
    const VendorCode& vendor,
    char* zoneNo,
    char* zoneType,
    const NationCode& nation,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(1, vendor);
  substParm(2, zoneNo);
  substParm(3, zoneType);
  substParm(4, nation);
  substParm(5, startDate);
  substParm(6, endDate);
  substParm(7, endDate);
  substParm(8, startDate);
  substParm(9, endDate);
  substParm(10, vendor);
  substParm(11, zoneNo);
  substParm(12, zoneType);
  substParm(13, nation);
  substParm(14, startDate);
  substParm(15, endDate);
  substParm(16, endDate);
  substParm(17, startDate);
  substParm(18, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstCC.push_back(QueryCheckNationAreaInZoneHistoricalSQLStatement<
        QueryCheckNationAreaInZoneHistorical>::mapRowToNationStateHistIsCurrChk(row));
  }
  LOG4CXX_INFO(_logger,
               "CHECKNATIONAREAINZONEHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryCheckNationAreaInZoneHistorical::isNationAreaInZone()

///////////////////////////////////////////////////////////
//  QueryCheckStateInSubArea
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryCheckStateInSubArea::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.CheckStateInSubArea"));
std::string QueryCheckStateInSubArea::_baseSQL;
bool QueryCheckStateInSubArea::_isInitialized = false;
SQLQueryInitializerHelper<QueryCheckStateInSubArea> g_CheckStateInSubArea;

const char*
QueryCheckStateInSubArea::getQueryName() const
{
  return "CHECKSTATEINSUBAREA";
};

void
QueryCheckStateInSubArea::initialize()
{
  if (!_isInitialized)
  {
    QueryCheckStateInSubAreaSQLStatement<QueryCheckStateInSubArea> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("CHECKSTATEINSUBAREA");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

bool
QueryCheckStateInSubArea::isStateInSubArea(const NationCode& nation,
                                           const StateCode& state,
                                           const LocCode& subArea)
{
  bool retVal = false;
  DBResultSet res(_dbAdapt);

  substParm(nation, 1);
  substParm(state, 2);
  substParm(subArea, 3);
  substCurrentDate();
  res.executeQuery(this);
  if (res.nextRow())
    retVal = true;
  res.freeResult();

  return retVal;
} // QueryCheckStateInSubArea::isStateInSubArea()

///////////////////////////////////////////////////////////
//  QueryCheckStateInSubAreaHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryCheckStateInSubAreaHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.CheckStateInSubAreaHistorical"));
std::string QueryCheckStateInSubAreaHistorical::_baseSQL;
bool QueryCheckStateInSubAreaHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryCheckStateInSubAreaHistorical> g_CheckStateInSubAreaHistorical;

const char*
QueryCheckStateInSubAreaHistorical::getQueryName() const
{
  return "CHECKSTATEINSUBAREAHISTORICAL";
};

void
QueryCheckStateInSubAreaHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryCheckStateInSubAreaHistoricalSQLStatement<QueryCheckStateInSubAreaHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("CHECKSTATEINSUBAREAHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryCheckStateInSubAreaHistorical::findStateInSubArea(
    std::vector<tse::NationStateHistIsCurrChk*>& lstCC,
    const NationCode& nation,
    const StateCode& state,
    const LocCode& subArea,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, nation);
  substParm(2, state);
  substParm(3, subArea);
  substParm(4, startDate);
  substParm(5, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstCC.push_back(QueryCheckStateInSubAreaHistoricalSQLStatement<
        QueryCheckStateInSubAreaHistorical>::mapRowToNationStateHistIsCurrChk(row));
  }
  LOG4CXX_INFO(_logger,
               "CHECKSTATEINSUBAREAHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryCheckStateInSubAreaHistorical::findStateInSubArea()

///////////////////////////////////////////////////////////
//  QueryCheckStateInArea
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryCheckStateInArea::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.CheckStateInArea"));
std::string QueryCheckStateInArea::_baseSQL;
bool QueryCheckStateInArea::_isInitialized = false;
SQLQueryInitializerHelper<QueryCheckStateInArea> g_CheckStateInArea;

const char*
QueryCheckStateInArea::getQueryName() const
{
  return "CHECKSTATEINAREA";
};

void
QueryCheckStateInArea::initialize()
{
  if (!_isInitialized)
  {
    QueryCheckStateInAreaSQLStatement<QueryCheckStateInArea> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("CHECKSTATEINAREA");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

bool
QueryCheckStateInArea::isStateInArea(const NationCode& nation,
                                     const StateCode& state,
                                     const LocCode& area)
{
  bool retVal = false;
  DBResultSet res(_dbAdapt);

  substParm(nation, 1);
  substParm(state, 2);
  substParm(area, 3);
  substCurrentDate();
  res.executeQuery(this);
  if (res.nextRow())
    retVal = true;
  res.freeResult();

  return retVal;
} // QueryCheckStateInArea::isStateInArea()

///////////////////////////////////////////////////////////
//  QueryCheckStateInAreaHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryCheckStateInAreaHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.CheckStateInAreaHistorical"));
std::string QueryCheckStateInAreaHistorical::_baseSQL;
bool QueryCheckStateInAreaHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryCheckStateInAreaHistorical> g_CheckStateInAreaHistorical;

const char*
QueryCheckStateInAreaHistorical::getQueryName() const
{
  return "CHECKSTATEINAREAHISTORICAL";
};

void
QueryCheckStateInAreaHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryCheckStateInAreaHistoricalSQLStatement<QueryCheckStateInAreaHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("CHECKSTATEINAREAHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryCheckStateInAreaHistorical::findStateInArea(std::vector<tse::NationStateHistIsCurrChk*>& lstCC,
                                                 const NationCode& nation,
                                                 const StateCode& state,
                                                 const LocCode& area,
                                                 const DateTime& startDate,
                                                 const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, nation);
  substParm(2, state);
  substParm(3, area);
  substParm(4, startDate);
  substParm(5, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstCC.push_back(QueryCheckStateInAreaHistoricalSQLStatement<
        QueryCheckStateInAreaHistorical>::mapRowToNationStateHistIsCurrChk(row));
  }

  LOG4CXX_INFO(_logger,
               "CHECKSTATEINAREAHISTORICAL: NumRows = " << res.numRows() << " Time = "
                                                        << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryCheckStateInAreaHistorical::findStateInArea()

///////////////////////////////////////////////////////////
//  QueryCheckStateInZone
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryCheckStateInZone::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.CheckStateInZone"));
std::string QueryCheckStateInZone::_baseSQL;
bool QueryCheckStateInZone::_isInitialized = false;
SQLQueryInitializerHelper<QueryCheckStateInZone> g_CheckStateInZone;

const char*
QueryCheckStateInZone::getQueryName() const
{
  return "CHECKSTATEINZONE";
};

void
QueryCheckStateInZone::initialize()
{
  if (!_isInitialized)
  {
    QueryCheckStateInZoneSQLStatement<QueryCheckStateInZone> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("CHECKSTATEINZONE");

    substTableDef(&_baseSQL);

    QueryCheckNationInZone::initialize();
    _isInitialized = true;
  }
} // initialize()

bool
QueryCheckStateInZone::isStateInZone(const VendorCode& vendor,
                                     int zoneNo,
                                     char zoneType,
                                     const NationCode& nation,
                                     const StateCode& state)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  std::string inclExcl = "";

  char znStr[15];
  sprintf(znStr, "%d", zoneNo);
  char zoneStr[] = "0000000";
  strcpy(&zoneStr[7 - strlen(znStr)], znStr);

  char zoneTypeStr[2];
  sprintf(zoneTypeStr, "%c", zoneType);

  substParm(vendor, 1);
  substParm(zoneStr, 2);
  substParm(zoneTypeStr, 3);
  substParm(nation, 4);
  substParm(state, 5);
  substCurrentDate();
  res.executeQuery(this);
  if ((row = res.nextRow()))
  {
    std::string inclExcl =
        QueryCheckStateInZoneSQLStatement<QueryCheckStateInZone>::mapRowToString(row);
    res.freeResult();
    if (inclExcl == "I")
      return true;
    else
      return false;
  }
  res.freeResult();

  QueryCheckNationInZone SQLNationInZone(_dbAdapt);
  return SQLNationInZone.isNationInZone(vendor, zoneNo, zoneType, nation);
} // QueryCheckStateInZone::isStateInZone()

///////////////////////////////////////////////////////////
//  QueryCheckStateInZoneHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryCheckStateInZoneHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.CheckStateInZoneHistorical"));
std::string QueryCheckStateInZoneHistorical::_baseSQL;
bool QueryCheckStateInZoneHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryCheckStateInZoneHistorical> g_CheckStateInZoneHistorical;

const char*
QueryCheckStateInZoneHistorical::getQueryName() const
{
  return "CHECKSTATEINZONEHISTORICAL";
};

void
QueryCheckStateInZoneHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryCheckStateInZoneHistoricalSQLStatement<QueryCheckStateInZoneHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("CHECKSTATEINZONEHISTORICAL");
    substTableDef(&_baseSQL);

    QueryCheckNationInZoneHistorical::initialize();
    _isInitialized = true;
  }
} // initialize()

void
QueryCheckStateInZoneHistorical::findStateInZone(std::vector<tse::NationStateHistIsCurrChk*>& lstCC,
                                                 const VendorCode& vendor,
                                                 int zoneNo,
                                                 char zoneType,
                                                 const NationCode& nation,
                                                 const StateCode& state,
                                                 const DateTime& startDate,
                                                 const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  char znStr[15];
  sprintf(znStr, "%d", zoneNo);
  char zoneStr[] = "0000000";
  strcpy(&zoneStr[7 - strlen(znStr)], znStr);

  char zoneTypeStr[2];
  sprintf(zoneTypeStr, "%c", zoneType);

  substParm(1, vendor);
  substParm(2, zoneStr);
  substParm(3, zoneTypeStr);
  substParm(4, nation);
  substParm(5, state);
  substParm(6, startDate);
  substParm(7, endDate);
  substParm(8, endDate);
  substParm(9, vendor);
  substParm(10, zoneStr);
  substParm(11, zoneTypeStr);
  substParm(12, nation);
  substParm(13, state);
  substParm(14, startDate);
  substParm(15, endDate);
  substParm(16, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  if ((row = res.nextRow()))
  {
    lstCC.push_back(QueryCheckStateInZoneHistoricalSQLStatement<
        QueryCheckStateInZoneHistorical>::mapRowToNationStateHistIsCurrChk(row));
  }
  LOG4CXX_INFO(_logger,
               "CHECKSTATEINZONEHISTORICAL: NumRows = " << res.numRows() << " Time = "
                                                        << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();

  QueryCheckNationInZoneHistorical SQLNationInZone(_dbAdapt);
  SQLNationInZone.findNationInZone(lstCC, vendor, zoneNo, zoneType, nation, startDate, endDate);
} // QueryCheckStateInZoneHistorical::findStateInZone()
}
