//----------------------------------------------------------------------------
//  File:           QueryGetTable989.cpp
//  Description:    QueryGetTable989
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
#include "DBAccess/Queries/QueryGetTable989.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetTable989SQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetTable989::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTable989"));
std::string QueryGetTable989::_baseSQL;
bool QueryGetTable989::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTable989> g_GetTable989;

const char*
QueryGetTable989::getQueryName() const
{
  return "GETTABLE989";
}

void
QueryGetTable989::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTable989SQLStatement<QueryGetTable989> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTABLE989");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTable989::findBaseFareRule(std::vector<const tse::BaseFareRule*>& table989s,
                                   VendorCode& vendor,
                                   int itemNo)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char itemStr[15];
  sprintf(itemStr, "%d", itemNo);

  substParm(vendor, 1);
  substParm(itemStr, 2);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    table989s.push_back(QueryGetTable989SQLStatement<QueryGetTable989>::mapRowToBaseFareRule(row));
  }
  LOG4CXX_INFO(_logger,
               "GETTABLE989: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                         << stopCPU() << ")");
  res.freeResult();
} // findBaseFareRule()

///////////////////////////////////////////////////////////
//  QueryGetTable989Historical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetTable989Historical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetTable989Historical"));
std::string QueryGetTable989Historical::_baseSQL;
bool QueryGetTable989Historical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTable989Historical> g_GetTable989Historical;

const char*
QueryGetTable989Historical::getQueryName() const
{
  return "GETTABLE989HISTORICAL";
}

void
QueryGetTable989Historical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTable989HistoricalSQLStatement<QueryGetTable989Historical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTABLE989HISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTable989Historical::findBaseFareRule(std::vector<const tse::BaseFareRule*>& table989s,
                                             VendorCode& vendor,
                                             int itemNo,
                                             const DateTime& startDate,
                                             const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char itemStr[15];
  sprintf(itemStr, "%d", itemNo);

  substParm(vendor, 1);
  substParm(itemStr, 2);
  substParm(3, startDate);
  substParm(4, endDate);
  substParm(vendor, 5);
  substParm(itemStr, 6);
  substParm(7, startDate);
  substParm(8, endDate);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    table989s.push_back(
        QueryGetTable989HistoricalSQLStatement<QueryGetTable989Historical>::mapRowToBaseFareRule(
            row));
  }
  LOG4CXX_INFO(_logger,
               "GETTABLE989HISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                   << " (" << stopCPU() << ")");
  res.freeResult();
} // findBaseFareRule()
}
