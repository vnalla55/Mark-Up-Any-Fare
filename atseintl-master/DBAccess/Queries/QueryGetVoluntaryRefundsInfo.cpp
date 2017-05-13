//-----------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly
// prohibited.
//-----------------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetVoluntaryRefundsInfo.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetVoluntaryRefundsInfoSQLStatement.h"

namespace tse
{

log4cxx::LoggerPtr
QueryGetVoluntaryRefundsInfo::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetVoluntaryRefundsInfo"));

std::string QueryGetVoluntaryRefundsInfo::_baseSQL;

bool QueryGetVoluntaryRefundsInfo::_isInitialized = false;

SQLQueryInitializerHelper<QueryGetVoluntaryRefundsInfo> g_GetVoluntaryRefundsInfo;

namespace
{
typedef QueryGetVoluntaryRefundsInfoSQLStatement<QueryGetVoluntaryRefundsInfo> SqlStatement;
typedef QueryGetVoluntaryRefundsInfoHistoricalSQLStatement<QueryGetVoluntaryRefundsInfoHistorical>
SqlStatementHistorical;
}

void
QueryGetVoluntaryRefundsInfo::initialize()
{
  if (!_isInitialized)
  {
    SqlStatement sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETVOLUNTARYREFUNDSINFO");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetVoluntaryRefundsInfo::find(std::vector<VoluntaryRefundsInfo*>& lst,
                                   const VendorCode& vendor,
                                   int itemNo)
{
  DBResultSet res(_dbAdapt);
  char strItemNo[20];
  sprintf(strItemNo, "%d", itemNo);

  substParm(vendor, 1);
  substParm(strItemNo, 2);
  substCurrentDate();

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  lst.reserve(res.numRows());
  while (Row* row = res.nextRow())
  {
    lst.push_back(SqlStatement::map(row));
  }

  LOG4CXX_INFO(_logger,
               "GETVOLUNTARYREFUNDS: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                 << " (" << stopCPU() << ")");

  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetVoluntaryRefundsInfoHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.QueryGetVoluntaryRefundsInfoHistorical"));
std::string QueryGetVoluntaryRefundsInfoHistorical::_baseSQL;
bool QueryGetVoluntaryRefundsInfoHistorical::_isInitialized = false;

SQLQueryInitializerHelper<QueryGetVoluntaryRefundsInfoHistorical>
g_GetVoluntaryRefundsInfoHistorical;

void
QueryGetVoluntaryRefundsInfoHistorical::initialize()
{
  if (!_isInitialized)
  {
    SqlStatementHistorical sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETVOLUNTARYREFUNDSINFOHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetVoluntaryRefundsInfoHistorical::find(std::vector<VoluntaryRefundsInfo*>& lst,
                                             const VendorCode& vendor,
                                             int itemNo,
                                             const DateTime& startDate,
                                             const DateTime& endDate)
{
  DBResultSet res(_dbAdapt);
  char itemStr[15];
  sprintf(itemStr, "%d", itemNo);

  substParm(vendor, 1);
  substParm(itemStr, 2);
  substParm(3, startDate);
  substParm(4, endDate);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  lst.reserve(lst.size() + res.numRows());
  while (Row* row = res.nextRow())
  {
    lst.push_back(SqlStatementHistorical::map(row));
  }

  LOG4CXX_INFO(_logger,
               "GETVOLUNTARYREFUNDSINFOHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");

  res.freeResult();
}
}
