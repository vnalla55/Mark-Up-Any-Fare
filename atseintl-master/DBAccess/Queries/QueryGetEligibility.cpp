//----------------------------------------------------------------------------
//  File:           QueryGetEligibility.cpp
//  Description:    QueryGetEligibility
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
#include "DBAccess/Queries/QueryGetEligibility.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetEligibilitySQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetEligibility::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetEligibility"));
std::string QueryGetEligibility::_baseSQL;
bool QueryGetEligibility::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetEligibility> g_GetEligibility;

const char*
QueryGetEligibility::getQueryName() const
{
  return "GETELIGIBILITY";
}

void
QueryGetEligibility::initialize()
{
  if (!_isInitialized)
  {
    QueryGetEligibilitySQLStatement<QueryGetEligibility> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETELIGIBILITY");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetEligibility::findEligibility(std::vector<const tse::EligibilityInfo*>& elges,
                                     const VendorCode& vendor,
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
    elges.push_back(
        QueryGetEligibilitySQLStatement<QueryGetEligibility>::mapRowToEligibilityInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETELIGIBILITY: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                            << stopCPU() << ")");
  res.freeResult();
} // findEligibility

///////////////////////////////////////////////////////////
//  QueryGetEligibilityHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetEligibilityHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetEligibilityHistorical"));
std::string QueryGetEligibilityHistorical::_baseSQL;
bool QueryGetEligibilityHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetEligibilityHistorical> g_GetEligibilityHistorical;

const char*
QueryGetEligibilityHistorical::getQueryName() const
{
  return "GETELIGIBILITYHISTORICAL";
}

void
QueryGetEligibilityHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetEligibilityHistoricalSQLStatement<QueryGetEligibilityHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETELIGIBILITYHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetEligibilityHistorical::findEligibility(std::vector<const tse::EligibilityInfo*>& elges,
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
    elges.push_back(QueryGetEligibilityHistoricalSQLStatement<
        QueryGetEligibilityHistorical>::mapRowToEligibilityInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETELIGIBILITYHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ")");
  res.freeResult();
} // findEligibility()
}
