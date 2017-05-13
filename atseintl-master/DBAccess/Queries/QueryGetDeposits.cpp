//----------------------------------------------------------------------------
//  File:           QueryGetDeposits.cpp
//  Description:    QueryGetDeposits
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
#include "DBAccess/Queries/QueryGetDeposits.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetDepositsSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetDeposits::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetDeposits"));
std::string QueryGetDeposits::_baseSQL;
bool QueryGetDeposits::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetDeposits> g_GetDeposits;

const char*
QueryGetDeposits::getQueryName() const
{
  return "GETDEPOSITS";
}

void
QueryGetDeposits::initialize()
{
  if (!_isInitialized)
  {
    QueryGetDepositsSQLStatement<QueryGetDeposits> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETDEPOSITS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetDeposits::findDeposits(std::vector<tse::Deposits*>& deposits,
                               const VendorCode& vendor,
                               int itemNo)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char itemStr[15];
  sprintf(itemStr, "%d", itemNo);

  substParm(1, vendor);
  substParm(2, itemStr);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    deposits.push_back(QueryGetDepositsSQLStatement<QueryGetDeposits>::mapRowToDeposits(row));
  }
  LOG4CXX_INFO(_logger,
               "GETDEPOSITS: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                         << stopCPU() << ")");
  res.freeResult();
} // findDeposits()

///////////////////////////////////////////////////////////
//  QueryGetDepositsHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetDepositsHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetDepositsHistorical"));
std::string QueryGetDepositsHistorical::_baseSQL;
bool QueryGetDepositsHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetDepositsHistorical> g_GetDepositsHistorical;

const char*
QueryGetDepositsHistorical::getQueryName() const
{
  return "GETDEPOSITSHISTORICAL";
}

void
QueryGetDepositsHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetDepositsHistoricalSQLStatement<QueryGetDepositsHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETDEPOSITSHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetDepositsHistorical::findDeposits(std::vector<tse::Deposits*>& deposits,
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
    deposits.push_back(
        QueryGetDepositsHistoricalSQLStatement<QueryGetDepositsHistorical>::mapRowToDeposits(row));
  }
  LOG4CXX_INFO(_logger,
               "GETDEPOSITSHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                   << " (" << stopCPU() << ")");
  res.freeResult();
} // findDeposits()
}
