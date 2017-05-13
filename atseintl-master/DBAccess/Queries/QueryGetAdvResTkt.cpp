//----------------------------------------------------------------------------
//  File:           QueryGetAdvResTkt.cpp
//  Description:    QueryGetAdvResTkt
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
#include "DBAccess/Queries/QueryGetAdvResTkt.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetAdvResTktSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetAdvResTkt::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAdvResTkt"));
std::string QueryGetAdvResTkt::_baseSQL;
bool QueryGetAdvResTkt::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAdvResTkt> g_GetAdvResTkt;

const char*
QueryGetAdvResTkt::getQueryName() const
{
  return "GETADVRESTKT";
}

void
QueryGetAdvResTkt::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAdvResTktSQLStatement<QueryGetAdvResTkt> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETADVRESTKT");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAdvResTkt::findAdvResTktInfo(std::vector<AdvResTktInfo*>& advres,
                                     const VendorCode& vendor,
                                     int itemNumber)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char itemStr[15];
  sprintf(itemStr, "%d", itemNumber);

  substParm(vendor, 1);
  substParm(itemStr, 2);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    advres.push_back(QueryGetAdvResTktSQLStatement<QueryGetAdvResTkt>::mapRowToAdvResTktInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETADVRESTKT: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                          << stopCPU() << ")");
  res.freeResult();
} // findAdvResTktInfo()

///////////////////////////////////////////////////////////
//  QueryGetAdvResTktHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAdvResTktHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetAdvResTktHistorical"));
std::string QueryGetAdvResTktHistorical::_baseSQL;
bool QueryGetAdvResTktHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAdvResTktHistorical> g_GetAdvResTktHistorical;

const char*
QueryGetAdvResTktHistorical::getQueryName() const
{
  return "GETADVRESTKTHISTORICAL";
}

void
QueryGetAdvResTktHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAdvResTktHistoricalSQLStatement<QueryGetAdvResTktHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETADVRESTKTHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAdvResTktHistorical::findAdvResTktInfo(std::vector<AdvResTktInfo*>& advres,
                                               const VendorCode& vendor,
                                               int itemNumber,
                                               const DateTime& startDate,
                                               const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char itemStr[15];
  sprintf(itemStr, "%d", itemNumber);

  substParm(vendor, 1);
  substParm(itemStr, 2);
  substParm(3, startDate);
  substParm(4, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    advres.push_back(
        QueryGetAdvResTktHistoricalSQLStatement<QueryGetAdvResTktHistorical>::mapRowToAdvResTktInfo(
            row));
  }
  LOG4CXX_INFO(_logger,
               "GETADVRESTKTHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                    << " (" << stopCPU() << ")");
  res.freeResult();
} // findAdvResTktInfo()
}
