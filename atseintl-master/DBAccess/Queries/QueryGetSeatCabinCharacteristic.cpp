//----------------------------------------------------------------------------
//  File:         QueryGetSeatCabinCharacteristic.cpp
//  Description:  QueryGetSeatCabinCharacteristic
//  Created:      10/10/2012
//  Authors:      Ram Papineni
//
//  Updates:
//
// � 2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetSeatCabinCharacteristic.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetSeatCabinCharacteristicSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetSeatCabinCharacteristic::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetSeatCabinCharacteristic"));
std::string QueryGetSeatCabinCharacteristic::_baseSQL;
bool QueryGetSeatCabinCharacteristic::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSeatCabinCharacteristic> g_GetSeatCabinCharacteristic;

const char*
QueryGetSeatCabinCharacteristic::getQueryName() const
{
  return "GETSEATCABINCHARACTERISTIC";
}

void
QueryGetSeatCabinCharacteristic::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSeatCabinCharacteristicSQLStatement<QueryGetSeatCabinCharacteristic> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSEATCABINCHARACTERISTIC");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetSeatCabinCharacteristic::findSeatCabinCharacteristic(
    std::vector<tse::SeatCabinCharacteristicInfo*>& sccInfoList,
    const CarrierCode& carrier,
    const Indicator& codeType)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, carrier);
  substParm(2, codeType);
  substCurrentDate();

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    sccInfoList.push_back(QueryGetSeatCabinCharacteristicSQLStatement<
        QueryGetSeatCabinCharacteristic>::mapRowToSeatCabinCharacteristicInfo(row));
  }

  LOG4CXX_INFO(_logger,
               "GETSEATCABINCHARACTERISTIC: NumRows: " << res.numRows() << " Time = " << stopTimer()
                                                       << " (" << stopCPU() << ") mSecs");
  res.freeResult();
} // findSeatCabinCharacteristic()

///////////////////////////////////////////////////////////
//  QueryGetSeatCabinCharacteristicHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetSeatCabinCharacteristicHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetSeatCabinCharacteristicHistorical"));
std::string QueryGetSeatCabinCharacteristicHistorical::_baseSQL;
bool QueryGetSeatCabinCharacteristicHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSeatCabinCharacteristicHistorical>
g_GetSeatCabinCharacteristicHistorical;

const char*
QueryGetSeatCabinCharacteristicHistorical::getQueryName() const
{
  return "GETSEATCABINCHARACTERISTICHISTORICAL";
}

void
QueryGetSeatCabinCharacteristicHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSeatCabinCharacteristicHistoricalSQLStatement<QueryGetSeatCabinCharacteristicHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSEATCABINCHARACTERISTICHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetSeatCabinCharacteristicHistorical::findSeatCabinCharacteristic(
    std::vector<tse::SeatCabinCharacteristicInfo*>& sccInfoList,
    const CarrierCode& carrier,
    const Indicator& codeType,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, carrier);
  substParm(2, codeType);
  substParm(3, startDate);
  substParm(4, endDate);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    sccInfoList.push_back(QueryGetSeatCabinCharacteristicSQLStatement<
        QueryGetSeatCabinCharacteristicHistorical>::mapRowToSeatCabinCharacteristicInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETSEATCABINCHARACTERISTICHISTORICAL: NumRows: "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ") mSecs");
  res.freeResult();
} // findSeatCabinCharacteristic()

///////////////////////////////////////////////////////////
//  QueryGetAllSeatCabinCharacteristic
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllSeatCabinCharacteristic::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllSeatCabinCharacteristic"));
std::string QueryGetAllSeatCabinCharacteristic::_baseSQL;
bool QueryGetAllSeatCabinCharacteristic::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllSeatCabinCharacteristic> g_GetAllSeatCabinCharacteristic;

const char*
QueryGetAllSeatCabinCharacteristic::getQueryName() const
{
  return "GETALLSEATCABINCHARACTERISTIC";
}

void
QueryGetAllSeatCabinCharacteristic::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllSeatCabinCharacteristicSQLStatement<QueryGetAllSeatCabinCharacteristic> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLSEATCABINCHARACTERISTIC");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
tse::QueryGetAllSeatCabinCharacteristic::findAllSeatCabinCharacteristic(
    std::vector<tse::SeatCabinCharacteristicInfo*>& sccInfoList)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    sccInfoList.push_back(QueryGetSeatCabinCharacteristicSQLStatement<
        QueryGetAllSeatCabinCharacteristic>::mapRowToSeatCabinCharacteristicInfo(row));
  }

  LOG4CXX_INFO(_logger,
               "GETALLSEATCABINCHARACTERISTIC: NumRows: "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ") mSecs");
  res.freeResult();
}
}
