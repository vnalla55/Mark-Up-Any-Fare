//----------------------------------------------------------------------------
//  File:           QueryGetTaxRestrictionLocation.cpp
//  Description:    QueryGetTaxRestrictionLocation
//  Created:        1/14/2010
//  Authors:        Piotr Badarycz
//
//  Updates:
//
// (c) 2010, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetTaxRestrictionLocation.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetTaxRestrictionLocationSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetTaxRestrictionLocation::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTaxRestrictionLocation"));
std::string QueryGetTaxRestrictionLocation::_baseSQL;
bool QueryGetTaxRestrictionLocation::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxRestrictionLocation> g_GetTaxRestrictionLocation;

const char*
QueryGetTaxRestrictionLocation::getQueryName() const
{
  return "GETTAXRESTRICTIONLOCATION";
}

void
QueryGetTaxRestrictionLocation::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxRestrictionLocationSQLStatement<QueryGetTaxRestrictionLocation> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXRESTRICTIONLOCATION");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxRestrictionLocation::findTaxRestrictionLocation(
    std::vector<const tse::TaxRestrictionLocationInfo*>& locations,
    TaxRestrictionLocation& location)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, location);
  substCurrentDate();

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  TaxRestrictionLocationInfo* prev = nullptr;
  TaxRestrictionLocationInfo* locationInfo = nullptr;
  while ((row = res.nextRow()))
  {
    locationInfo = QueryGetTaxRestrictionLocationSQLStatement<
        QueryGetTaxRestrictionLocation>::mapRowToTaxRestrictionLocation(row, prev);
    if (locationInfo != prev)
      locations.push_back(locationInfo);
    prev = locationInfo;
  }
  LOG4CXX_INFO(_logger,
               "GETTAXRESTRICTIONLOCATION: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                       << " msecs");
  res.freeResult();
} // findTaxRestrictionLocation()

///////////////////////////////////////////////////////////
//
//  QueryGetTaxRestrictionLocationHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetTaxRestrictionLocationHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTaxRestrictionLocationHistorical"));
std::string QueryGetTaxRestrictionLocationHistorical::_baseSQL;
bool QueryGetTaxRestrictionLocationHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxRestrictionLocationHistorical>
g_GetTaxRestrictionLocationHistorical;

const char*
QueryGetTaxRestrictionLocationHistorical::getQueryName() const
{
  return "GETTAXRESTRICTIONLOCATIONHISTORICAL";
}

void
QueryGetTaxRestrictionLocationHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxRestrictionLocationHistoricalSQLStatement<QueryGetTaxRestrictionLocationHistorical>
    sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXRESTRICTIONLOCATIONHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxRestrictionLocationHistorical::findTaxRestrictionLocation(
    std::vector<const tse::TaxRestrictionLocationInfo*>& locations,
    TaxRestrictionLocation& location,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, location);
  substParm(2, startDate);
  substParm(3, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  TaxRestrictionLocationInfo* prev = nullptr;

  while ((row = res.nextRow()))
  {
    TaxRestrictionLocationInfo* locationInfo = QueryGetTaxRestrictionLocationHistoricalSQLStatement<
        QueryGetTaxRestrictionLocationHistorical>::mapRowToTaxRestrictionLocation(row, prev);
    if (locationInfo != prev)
      locations.push_back(locationInfo);
    prev = locationInfo;
  }
  LOG4CXX_INFO(_logger,
               "GETTAXRESTRICTIONLOCATIONHISTORICAL: NumRows = " << res.numRows() << " Time = "
                                                                 << stopTimer() << " msecs");
  res.freeResult();
} // findTaxRestrictionLocation()
}
