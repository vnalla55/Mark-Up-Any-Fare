//----------------------------------------------------------------------------
//  File:           QueryGetSvcFeesTicketDesignator.cpp
//  Description:    QuerySvcFeesTicketDesignator
//  Created:        3/11/2009
// Authors:
//
//  Updates:
//
//  2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetSvcFeesTicketDesignator.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetSvcFeesTicketDesignatorSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetSvcFeesTicketDesignator::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetSvcFeesTicketDesignator"));
std::string QueryGetSvcFeesTicketDesignator::_baseSQL;
bool QueryGetSvcFeesTicketDesignator::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSvcFeesTicketDesignator> g_GetSvcFeesTicketDesignator;

const char*
QueryGetSvcFeesTicketDesignator::getQueryName() const
{
  return "GETSVCFEESTICKETDESIGNATOR";
}

void
QueryGetSvcFeesTicketDesignator::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSvcFeesTicketDesignatorSQLStatement<QueryGetSvcFeesTicketDesignator> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSVCFEESTICKETDESIGNATOR");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetSvcFeesTicketDesignator::findSvcFeesTktDesignatorInfo(
    std::vector<tse::SvcFeesTktDesignatorInfo*>& tktD, const VendorCode& vendor, int itemNo)
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
    tktD.push_back(QueryGetSvcFeesTicketDesignatorSQLStatement<
        QueryGetSvcFeesTicketDesignator>::mapRowToSvcFeesTktDesignatorInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETSVCFEESTICKETDESIGNATOR: NumRows " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ") mSecs");
  res.freeResult();
} // findSvcFeesTicketDesignatorInfo()

///////////////////////////////////////////////////////////
//  QueryGetSvcFeesTicketDesignatorHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetSvcFeesTicketDesignatorHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.QueryGetSvcFeesTicketDesignatorHistorical"));
std::string QueryGetSvcFeesTicketDesignatorHistorical::_baseSQL;
bool QueryGetSvcFeesTicketDesignatorHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSvcFeesTicketDesignatorHistorical>
g_GetSvcFeesTicketDesignatorHistorical;

const char*
QueryGetSvcFeesTicketDesignatorHistorical::getQueryName() const
{
  return "GETSVCFEESTICKETDESIGNATORHISTORICAL";
}

void
QueryGetSvcFeesTicketDesignatorHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSvcFeesTicketDesignatorHistoricalSQLStatement<QueryGetSvcFeesTicketDesignatorHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSVCFEESTICKETDESIGNATORHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetSvcFeesTicketDesignatorHistorical::findSvcFeesTktDesignatorInfo(
    std::vector<tse::SvcFeesTktDesignatorInfo*>& tktD,
    const VendorCode& vendor,
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
    tktD.push_back(QueryGetSvcFeesTicketDesignatorHistoricalSQLStatement<
        QueryGetSvcFeesTicketDesignatorHistorical>::mapRowToSvcFeesTktDesignatorInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETSVCFEESTICKETDESIGNATORHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findSvcFeesTicketDesignatorInfo()
}
