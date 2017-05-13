//----------------------------------------------------------------------------
//  File:           QueryGetMultiTransport.cpp
//  Description:    QueryGetMultiTransport
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

#include "DBAccess/Queries/QueryGetMultiTransport.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetMultiTransportSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetMultiTransport::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetMultiTransport"));
std::string QueryGetMultiTransport::_baseSQL;
bool QueryGetMultiTransport::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMultiTransport> g_GetMultiTransport;

const char*
QueryGetMultiTransport::getQueryName() const
{
  return "GETMULTITRANSPORT";
}

void
QueryGetMultiTransport::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMultiTransportSQLStatement<QueryGetMultiTransport> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMULTITRANSPORT");

    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMultiTransport::findMultiTransport(std::vector<tse::MultiTransport*>& multiTransport,
                                           const LocCode& loc)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, loc);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    multiTransport.push_back(
        QueryGetMultiTransportSQLStatement<QueryGetMultiTransport>::mapRowToMultiTransport(row));
  }
  LOG4CXX_INFO(_logger,
               "GETMULTITRANSPORT: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                               << stopCPU() << ")");
  res.freeResult();
} // findMultiTransport()

///////////////////////////////////////////////////////////
//
//  QueryGetAllMultiTransport
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllMultiTransport::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllMultiTransport"));
std::string QueryGetAllMultiTransport::_baseSQL;
bool QueryGetAllMultiTransport::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllMultiTransport> g_GetAllMultiTransport;

const char*
QueryGetAllMultiTransport::getQueryName() const
{
  return "GETALLMULTITRANSPORT";
}

void
QueryGetAllMultiTransport::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllMultiTransportSQLStatement<QueryGetAllMultiTransport> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLMULTITRANSPORT");

    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllMultiTransport::findAllMultiTransport(std::vector<tse::MultiTransport*>& multiTransport)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    multiTransport.push_back(
        QueryGetAllMultiTransportSQLStatement<QueryGetAllMultiTransport>::mapRowToMultiTransport(
            row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLMULTITRANSPORT: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                  << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllMultiTransport()

///////////////////////////////////////////////////////////
//
//  QueryGetMultiTransportCity
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetMultiTransportCity::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetMultiTransportCity"));
std::string QueryGetMultiTransportCity::_baseSQL;
bool QueryGetMultiTransportCity::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMultiTransportCity> g_GetMultiTransportCity;

const char*
QueryGetMultiTransportCity::getQueryName() const
{
  return "GETMULTITRANSPORTCITY";
}

void
QueryGetMultiTransportCity::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMultiTransportCitySQLStatement<QueryGetMultiTransportCity> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMULTITRANSPORTCITY");

    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMultiTransportCity::findMultiTransportCity(
    std::vector<tse::MultiTransport*>& multiTransport, const LocCode& loc)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, loc);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    multiTransport.push_back(
        QueryGetMultiTransportCitySQLStatement<QueryGetMultiTransportCity>::mapRowToMultiTransport(
            row));
  }
  LOG4CXX_INFO(_logger,
               "GETMULTITRANSPORTCITY: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                   << " (" << stopCPU() << ")");
  res.freeResult();
} // findMultiTransportCity()

///////////////////////////////////////////////////////////
//
//  QueryGetAllMultiTransportCity
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllMultiTransportCity::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllMultiTransportCity"));
std::string QueryGetAllMultiTransportCity::_baseSQL;
bool QueryGetAllMultiTransportCity::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllMultiTransportCity> g_GetAllMultiTransportCity;

const char*
QueryGetAllMultiTransportCity::getQueryName() const
{
  return "GETALLMULTITRANSPORTCITY";
}

void
QueryGetAllMultiTransportCity::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllMultiTransportCitySQLStatement<QueryGetAllMultiTransportCity> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLMULTITRANSPORTCITY");

    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllMultiTransportCity::findAllMultiTransportCity(
    std::vector<tse::MultiTransport*>& multiTransport)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    multiTransport.push_back(QueryGetAllMultiTransportCitySQLStatement<
        QueryGetAllMultiTransportCity>::mapRowToMultiTransport(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLMULTITRANSPORTCITY: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllMultiTransportCity()

///////////////////////////////////////////////////////////
//
//  QueryGetMultiTransportLocs
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetMultiTransportLocs::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetMultiTransportLocs"));
std::string QueryGetMultiTransportLocs::_baseSQL;
bool QueryGetMultiTransportLocs::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMultiTransportLocs> g_GetMultiTransportLocs;

const char*
QueryGetMultiTransportLocs::getQueryName() const
{
  return "GETMULTITRANSPORTLOCS";
}

void
QueryGetMultiTransportLocs::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMultiTransportLocsSQLStatement<QueryGetMultiTransportLocs> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMULTITRANSPORTLOCS");

    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMultiTransportLocs::findMultiTransportLocs(
    std::vector<tse::MultiTransport*>& multiTransport, const LocCode& loc)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, loc);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    multiTransport.push_back(
        QueryGetMultiTransportLocsSQLStatement<QueryGetMultiTransportLocs>::mapRowToMultiTransport(
            row));
  }
  LOG4CXX_INFO(_logger,
               "GETMULTITRANSPORTCITY: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                   << " (" << stopCPU() << ")");
  res.freeResult();
} // findMultiTransportLocs()

///////////////////////////////////////////////////////////
//
//  QueryGetAllMultiTransportLocs
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllMultiTransportLocs::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllMultiTransportLocs"));
std::string QueryGetAllMultiTransportLocs::_baseSQL;
bool QueryGetAllMultiTransportLocs::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllMultiTransportLocs> g_GetAllMultiTransportLocs;

const char*
QueryGetAllMultiTransportLocs::getQueryName() const
{
  return "GETALLMULTITRANSPORTLOCS";
}

void
QueryGetAllMultiTransportLocs::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllMultiTransportLocsSQLStatement<QueryGetAllMultiTransportLocs> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLMULTITRANSPORTLOCS");

    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllMultiTransportLocs::findAllMultiTransportLocs(
    std::vector<tse::MultiTransport*>& multiTransport)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    multiTransport.push_back(QueryGetAllMultiTransportLocsSQLStatement<
        QueryGetAllMultiTransportLocs>::mapRowToMultiTransport(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLMULTITRANSPORTLOCS: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllMultiTransportLocs()

///////////////////////////////////////////////////////////
//
//  QueryGetMultiTransportLocsHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetMultiTransportLocsHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetMultiTransportLocsHistorical"));
std::string QueryGetMultiTransportLocsHistorical::_baseSQL;
bool QueryGetMultiTransportLocsHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMultiTransportLocsHistorical> g_GetMultiTransportLocsHistorical;

const char*
QueryGetMultiTransportLocsHistorical::getQueryName() const
{
  return "GETMULTITRANSPORTLOCSHISTORICAL";
}

void
QueryGetMultiTransportLocsHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMultiTransportLocsHistoricalSQLStatement<QueryGetMultiTransportLocsHistorical>
    sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMULTITRANSPORTLOCSHISTORICAL");

    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMultiTransportLocsHistorical::findMultiTransportLocsHistorical(
    std::vector<tse::MultiTransport*>& multiTransport,
    const LocCode& loc,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, loc);
  substParm(2, startDate);
  substParm(3, endDate);
  substParm(4, endDate);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    multiTransport.push_back(QueryGetMultiTransportLocsHistoricalSQLStatement<
        QueryGetMultiTransportLocsHistorical>::mapRowToMultiTransport(row));
  }
  LOG4CXX_INFO(_logger,
               "GETMULTITRANSPORTCITYHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllMultiTransportLocs()

///////////////////////////////////////////////////////////
//
//  QueryGetMultiTransportHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetMultiTransportHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetMultiTransportHistorical"));
std::string QueryGetMultiTransportHistorical::_baseSQL;
bool QueryGetMultiTransportHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMultiTransportHistorical> g_GetMultiTransportHistorical;

const char*
QueryGetMultiTransportHistorical::getQueryName() const
{
  return "GETMULTITRANSPORTHISTORICAL";
}

void
QueryGetMultiTransportHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMultiTransportHistoricalSQLStatement<QueryGetMultiTransportHistorical> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMULTITRANSPORTHISTORICAL");

    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMultiTransportHistorical::findMultiTransport(
    std::vector<tse::MultiTransport*>& multiTransport,
    const LocCode& loc,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, loc);
  substParm(2, startDate);
  substParm(3, endDate);
  substParm(4, endDate);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    multiTransport.push_back(QueryGetMultiTransportHistoricalSQLStatement<
        QueryGetMultiTransportHistorical>::mapRowToMultiTransport(row));
  }
  LOG4CXX_INFO(_logger,
               "GETMULTITRANSPORTHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findMultiTransport()

///////////////////////////////////////////////////////////
//
//  QueryGetMultiTransportCityHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetMultiTransportCityHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetMultiTransportCityHistorical"));
std::string QueryGetMultiTransportCityHistorical::_baseSQL;
bool QueryGetMultiTransportCityHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMultiTransportCityHistorical> g_GetMultiTransportCityHistorical;

const char*
QueryGetMultiTransportCityHistorical::getQueryName() const
{
  return "GETMULTITRANSPORTCITYHISTORICAL";
}

void
QueryGetMultiTransportCityHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMultiTransportCityHistoricalSQLStatement<QueryGetMultiTransportCityHistorical>
    sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMULTITRANSPORTCITYHISTORICAL");

    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMultiTransportCityHistorical::findMultiTransportCity(
    std::vector<tse::MultiTransport*>& multiTransport,
    const LocCode& loc,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, loc);
  substParm(2, startDate);
  substParm(3, endDate);
  substParm(4, endDate);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    multiTransport.push_back(QueryGetMultiTransportCityHistoricalSQLStatement<
        QueryGetMultiTransportCityHistorical>::mapRowToMultiTransport(row));
  }
  LOG4CXX_INFO(_logger,
               "GETMULTITRANSPORTCITY: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                   << " (" << stopCPU() << ")");
  res.freeResult();
} // findMultiTransportCity()
}
