//----------------------------------------------------------------------------
//  File:           QueryGetTicketEndorsementsInfo.cpp
//  Description:    QueryGetTicketEndorsementsInfo
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
#include "DBAccess/Queries/QueryGetTicketEndorsementsInfo.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetTicketEndorsementsInfoSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetTicketEndorsementsInfo::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTicketEndorsementsInfo"));
std::string QueryGetTicketEndorsementsInfo::_baseSQL;
bool QueryGetTicketEndorsementsInfo::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTicketEndorsementsInfo> g_GetTicketEndorsementsInfo;

const char*
QueryGetTicketEndorsementsInfo::getQueryName() const
{
  return "GETTICKETENDORSEMENTSINFO";
}

void
QueryGetTicketEndorsementsInfo::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTicketEndorsementsInfoSQLStatement<QueryGetTicketEndorsementsInfo> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTICKETENDORSEMENTSINFO");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTicketEndorsementsInfo::findTicketEndorsementsInfo(
    std::vector<tse::TicketEndorsementsInfo*>& lstTEI, VendorCode& vendor, int itemNo)
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
    lstTEI.push_back(QueryGetTicketEndorsementsInfoSQLStatement<
        QueryGetTicketEndorsementsInfo>::mapRowToTicketEndorsementsInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETTICKETENDORSEMENTSINFO: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                       << " (" << stopCPU() << ")");
  res.freeResult();
} // findTicketEndorsementsInfo()

///////////////////////////////////////////////////////////
//  QueryGetTicketEndorsementsInfoHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetTicketEndorsementsInfoHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.QueryGetTicketEndorsementsInfoHistorical"));
std::string QueryGetTicketEndorsementsInfoHistorical::_baseSQL;
bool QueryGetTicketEndorsementsInfoHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTicketEndorsementsInfoHistorical>
g_GetTicketEndorsementsInfoHistorical;

const char*
QueryGetTicketEndorsementsInfoHistorical::getQueryName() const
{
  return "GETTICKETENDORSEMENTSINFOHISTORICAL";
}

void
QueryGetTicketEndorsementsInfoHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTicketEndorsementsInfoHistoricalSQLStatement<QueryGetTicketEndorsementsInfoHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTICKETENDORSEMENTSINFOHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTicketEndorsementsInfoHistorical::findTicketEndorsementsInfo(
    std::vector<tse::TicketEndorsementsInfo*>& lstTEI,
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
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstTEI.push_back(QueryGetTicketEndorsementsInfoHistoricalSQLStatement<
        QueryGetTicketEndorsementsInfoHistorical>::mapRowToTicketEndorsementsInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETTICKETENDORSEMENTSINFOHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findTicketEndorsementsInfo()
}
