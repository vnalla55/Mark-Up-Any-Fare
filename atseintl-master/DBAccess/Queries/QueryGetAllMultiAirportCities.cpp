//----------------------------------------------------------------------------
//  File:           QueryGetAllMultiAirportCities.cpp
//  Description:    QueryGetAllMultiAirportCities
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

#include "DBAccess/Queries/QueryGetAllMultiAirportCities.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetAllMultiAirportCitiesSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetMultiAirportCity::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetMultiAirportCity"));
std::string QueryGetMultiAirportCity::_baseSQL;
bool QueryGetMultiAirportCity::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMultiAirportCity> g_GetMultiAirportCity;

const char*
QueryGetMultiAirportCity::getQueryName() const
{
  return "GETMULTIAIRPORTCITY";
}

void
QueryGetMultiAirportCity::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMultiAirportCitySQLStatement<QueryGetMultiAirportCity> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMULTIAIRPORTCITY");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMultiAirportCity::findMultiAirportCity(
    std::vector<tse::MultiAirportCity*>& multiAirportCities, const LocCode& airportCode)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(airportCode, 1);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {

    multiAirportCities.push_back(
        QueryGetMultiAirportCitySQLStatement<QueryGetMultiAirportCity>::mapRowToMultiAirportCity(
            row));
  }
  LOG4CXX_INFO(_logger,
               "GETMULTIAIRPORTCITY: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                 << " (" << stopCPU() << ")");
  res.freeResult();
} // findMultiAirportCity()

///////////////////////////////////////////////////////////
//
//  QueryGetAllMultiAirportCities
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllMultiAirportCities::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllMultiAirportCities"));
std::string QueryGetAllMultiAirportCities::_baseSQL;
bool QueryGetAllMultiAirportCities::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllMultiAirportCities> g_GetAllMultiAirportCities;

const char*
QueryGetAllMultiAirportCities::getQueryName() const
{
  return "GETALLMULTIAIRPORTCITIES";
}

void
QueryGetAllMultiAirportCities::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllMultiAirportCitiesSQLStatement<QueryGetAllMultiAirportCities> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLMULTIAIRPORTCITIES");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllMultiAirportCities::findAllMultiAirportCities(
    std::vector<tse::MultiAirportCity*>& multiAirportCities)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    multiAirportCities.push_back(QueryGetAllMultiAirportCitiesSQLStatement<
        QueryGetAllMultiAirportCities>::mapRowToMultiAirportCity(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLMULTIAIRPORTCITIES: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllMultiAirportCities()

///////////////////////////////////////////////////////////
//
//  QueryGetAllMultiAirportsByCity
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllMultiAirportsByCity::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllMultiAirportsByCity"));
std::string QueryGetAllMultiAirportsByCity::_baseSQL;
bool QueryGetAllMultiAirportsByCity::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllMultiAirportsByCity> g_GetAllMultiAirportsByCity;

const char*
QueryGetAllMultiAirportsByCity::getQueryName() const
{
  return "GETALLMULTIAIRPORTSBYCITY";
}

void
QueryGetAllMultiAirportsByCity::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllMultiAirportsByCitySQLStatement<QueryGetAllMultiAirportsByCity> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLMULTIAIRPORTSBYCITY");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllMultiAirportsByCity::findAllMultiAirportsByCity(
    std::vector<tse::MultiAirportsByCity*>& multiAirportsByCity)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    multiAirportsByCity.push_back(QueryGetAllMultiAirportsByCitySQLStatement<
        QueryGetAllMultiAirportsByCity>::mapRowToMultiAirportsByCity(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLMULTIAIRPORTSBYCITY: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                       << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllMultiAirportsByCity()
}
