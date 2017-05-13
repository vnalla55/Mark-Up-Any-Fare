//----------------------------------------------------------------------------
//  File:           QueryGetMarriedCabin.cpp
//  Description:    QueryGetMarriedCabin
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

#include "DBAccess/Queries/QueryGetMarriedCabin.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetMarriedCabinSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetMarriedCabin::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetMarriedCabin"));
std::string QueryGetMarriedCabin::_baseSQL;
bool QueryGetMarriedCabin::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMarriedCabin> g_GetMarriedCabin;

const char*
QueryGetMarriedCabin::getQueryName() const
{
  return "GETMARRIEDCABIN";
}

void
QueryGetMarriedCabin::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMarriedCabinSQLStatement<QueryGetMarriedCabin> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMARRIEDCABIN");

    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMarriedCabin::findMarriedCabins(std::vector<MarriedCabin*>& mcList,
                                        const CarrierCode& carrier,
                                        const BookingCode& premiumCabin)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, carrier);
  substParm(2, premiumCabin);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  MarriedCabin* prev = nullptr;
  while ((row = res.nextRow()))
  {
    MarriedCabin* rec =
        QueryGetMarriedCabinSQLStatement<QueryGetMarriedCabin>::mapRowToMarriedCabin(row, prev);
    if (prev != rec)
    {
      mcList.push_back(rec);
      prev = rec;
    }
  }
  LOG4CXX_INFO(_logger,
               "GETMARRIEDCABIN: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                             << " msecs");
  res.freeResult();
} // findMarriedCabins()

///////////////////////////////////////////////////////////
//
//  QueryGetAllMarriedCabinHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllMarriedCabinHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllMarriedCabinHistorical"));
std::string QueryGetAllMarriedCabinHistorical::_baseSQL;
bool QueryGetAllMarriedCabinHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllMarriedCabinHistorical> g_GetAllMarriedCabinHistorical;

const char*
QueryGetAllMarriedCabinHistorical::getQueryName() const
{
  return "GETALLMARRIEDCABINHISTORICAL";
}

void
QueryGetAllMarriedCabinHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllMarriedCabinHistoricalSQLStatement<QueryGetAllMarriedCabinHistorical> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLMARRIEDCABINHISTORICAL");

    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllMarriedCabinHistorical::findAllMarriedCabins(std::vector<MarriedCabin*>& mcList)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  MarriedCabin* prev = nullptr;
  while ((row = res.nextRow()))
  {
    MarriedCabin* rec = QueryGetMarriedCabinHistoricalSQLStatement<
        QueryGetMarriedCabinHistorical>::mapRowToMarriedCabin(row, prev);
    if (prev != rec)
    {
      mcList.push_back(rec);
      prev = rec;
    }
  }
  LOG4CXX_INFO(_logger,
               "GETALLMARRIEDCABINHISTORICAL: NumRows = " << res.numRows()
                                                          << " Time = " << stopTimer() << " msecs");
  res.freeResult();
} // findAllMarriedCabins()

///////////////////////////////////////////////////////////
//
//  QueryGetMarriedCabinHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetMarriedCabinHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetMarriedCabinHistorical"));
std::string QueryGetMarriedCabinHistorical::_baseSQL;
bool QueryGetMarriedCabinHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMarriedCabinHistorical> g_GetMarriedCabinHistorical;

const char*
QueryGetMarriedCabinHistorical::getQueryName() const
{
  return "GETMARRIEDCABINHISTORICAL";
}

void
QueryGetMarriedCabinHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMarriedCabinHistoricalSQLStatement<QueryGetMarriedCabinHistorical> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMARRIEDCABINHISTORICAL");

    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMarriedCabinHistorical::findMarriedCabins(std::vector<MarriedCabin*>& mcList,
                                                  const CarrierCode& carrier,
                                                  const BookingCode& premiumCabin)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, carrier);
  substParm(2, premiumCabin);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  MarriedCabin* prev = nullptr;
  while ((row = res.nextRow()))
  {
    MarriedCabin* rec = QueryGetMarriedCabinHistoricalSQLStatement<
        QueryGetMarriedCabinHistorical>::mapRowToMarriedCabin(row, prev);
    if (prev != rec)
    {
      mcList.push_back(rec);
      prev = rec;
    }
  }
  LOG4CXX_INFO(_logger,
               "GETMARRIEDCABINHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                       << " msecs");
  res.freeResult();
} // findMarriedCabins()
}
