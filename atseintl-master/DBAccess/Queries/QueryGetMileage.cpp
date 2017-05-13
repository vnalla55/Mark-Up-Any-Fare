//----------------------------------------------------------------------------
//  File:           QueryGetMileage.cpp
//  Description:    QueryGetMileage
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
#include "DBAccess/Queries/QueryGetMileage.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetMileageSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetMileage::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetMileage"));
std::string QueryGetMileage::_baseSQL;
bool QueryGetMileage::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMileage> g_GetMileage;

const char*
QueryGetMileage::getQueryName() const
{
  return "GETMILEAGE";
}

void
QueryGetMileage::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMileageSQLStatement<QueryGetMileage> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMILEAGE");

    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMileage::findMileage(std::vector<tse::Mileage*>& mileages,
                             const LocCode& orig,
                             const LocCode& dest,
                             const Indicator mileType)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(orig, 1);
  substParm(dest, 2);
  substParm(3, mileType);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    mileages.push_back(QueryGetMileageSQLStatement<QueryGetMileage>::mapRowToMileage(row));
  }
  LOG4CXX_INFO(_logger,
               "GETMILEAGE: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                        << stopCPU() << ")");
  res.freeResult();
} // findMileage()

///////////////////////////////////////////////////////////
//  QueryGetMileageHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetMileageHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetMileageHistorical"));
std::string QueryGetMileageHistorical::_baseSQL;
bool QueryGetMileageHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMileageHistorical> g_GetMileageHistorical;

const char*
QueryGetMileageHistorical::getQueryName() const
{
  return "GETMILEAGEHISTORICAL";
}

void
QueryGetMileageHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMileageHistoricalSQLStatement<QueryGetMileageHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMILEAGEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMileageHistorical::findMileage(std::vector<tse::Mileage*>& mileages,
                                       const LocCode& orig,
                                       const LocCode& dest,
                                       const Indicator mileType,
                                       const DateTime& startDate,
                                       const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(orig, 1);
  substParm(dest, 2);
  substParm(3, mileType);
  substParm(4, startDate);
  substParm(5, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    mileages.push_back(
        QueryGetMileageHistoricalSQLStatement<QueryGetMileageHistorical>::mapRowToMileage(row));
  }
  LOG4CXX_INFO(_logger,
               "GETMILEAGEHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                  << " (" << stopCPU() << ")");
  res.freeResult();
} // findMileage()
}
