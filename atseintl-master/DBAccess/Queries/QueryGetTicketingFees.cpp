//----------------------------------------------------------------------------
//  File:           QueryGetTicketingFees.cpp
//  Description:    QueryTicketingFees
//  Created:        2/26/2009
// Authors:
//
//  Updates:
//
//  2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetTicketingFees.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetTicketingFeesSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetTicketingFees::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTicketingFees"));
std::string QueryGetTicketingFees::_baseSQL;
bool QueryGetTicketingFees::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTicketingFees> g_GetTicketingFees;

const char*
QueryGetTicketingFees::getQueryName() const
{
  return "GETTICKETINGFEES";
}

void
QueryGetTicketingFees::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTicketingFeesSQLStatement<QueryGetTicketingFees> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTICKETINGFEES");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTicketingFees::findTicketingFeesInfo(std::vector<tse::TicketingFeesInfo*>& tktFees,
                                             const VendorCode& vendor,
                                             const CarrierCode& carrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(vendor, 1);
  substParm(carrier, 2);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    tktFees.push_back(
        QueryGetTicketingFeesSQLStatement<QueryGetTicketingFees>::mapRowToTicketingFeesInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETTICKETINGFEES: NumRows " << res.numRows() << " Time = " << stopTimer() << " ("
                                            << stopCPU() << ") mSecs");
  res.freeResult();
} // findTicketingFees()

///////////////////////////////////////////////////////////
//  QueryGetTicketingFeesHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetTicketingFeesHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetTicketingFeesHistorical"));
std::string QueryGetTicketingFeesHistorical::_baseSQL;
bool QueryGetTicketingFeesHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTicketingFeesHistorical> g_GetTicketingFeesHistorical;

const char*
QueryGetTicketingFeesHistorical::getQueryName() const
{
  return "GETTICKETINGFEESHISTORICAL";
}

void
QueryGetTicketingFeesHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTicketingFeesHistoricalSQLStatement<QueryGetTicketingFeesHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTICKETINGFEESHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTicketingFeesHistorical::findTicketingFeesInfo(
    std::vector<tse::TicketingFeesInfo*>& tktFees,
    const VendorCode& vendor,
    const CarrierCode& carrier,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(vendor, 1);
  substParm(carrier, 2);
  substParm(3, startDate);
  substParm(4, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    tktFees.push_back(QueryGetTicketingFeesHistoricalSQLStatement<
        QueryGetTicketingFeesHistorical>::mapRowToTicketingFeesInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETTICKETINGFEESHISTORICAL: NumRows = " << res.numRows() << " Time = "
                                                        << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findTicketingFeesInfo()
}
