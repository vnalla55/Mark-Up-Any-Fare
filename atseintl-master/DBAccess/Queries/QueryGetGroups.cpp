//----------------------------------------------------------------------------
//  File:           QueryGetGroups.cpp
//  Description:    QueryGetGroups
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
#include "DBAccess/Queries/QueryGetGroups.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetGroupsSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetGroups::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetGroups"));
std::string QueryGetGroups::_baseSQL;
bool QueryGetGroups::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetGroups> g_GetGroups;

const char*
QueryGetGroups::getQueryName() const
{
  return "GETGROUPS";
}

void
QueryGetGroups::initialize()
{
  if (!_isInitialized)
  {
    QueryGetGroupsSQLStatement<QueryGetGroups> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETGROUPS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetGroups::findGroups(std::vector<tse::Groups*>& groups, const VendorCode& vendor, int itemNo)
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
    groups.push_back(QueryGetGroupsSQLStatement<QueryGetGroups>::mapRowToGroups(row));
  }
  LOG4CXX_INFO(_logger,
               "GETGROUPS: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                       << stopCPU() << ")");
  res.freeResult();
} // findGroups()

///////////////////////////////////////////////////////////
//  QueryGetGroupsHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetGroupsHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetGroupsHistorical"));
std::string QueryGetGroupsHistorical::_baseSQL;
bool QueryGetGroupsHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetGroupsHistorical> g_GetGroupsHistorical;

const char*
QueryGetGroupsHistorical::getQueryName() const
{
  return "GETGROUPSHISTORICAL";
}

void
QueryGetGroupsHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetGroupsHistoricalSQLStatement<QueryGetGroupsHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETGROUPSHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetGroupsHistorical::findGroups(std::vector<tse::Groups*>& groups,
                                     const VendorCode& vendor,
                                     int itemNo,
                                     const DateTime& startDate,
                                     const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char itemStr[15];
  sprintf(itemStr, "%d", itemNo);

  substParm(1, vendor);
  substParm(2, itemStr);
  substParm(3, startDate);
  substParm(4, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    groups.push_back(
        QueryGetGroupsHistoricalSQLStatement<QueryGetGroupsHistorical>::mapRowToGroups(row));
  }
  LOG4CXX_INFO(_logger,
               "GETGROUPSHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                 << " (" << stopCPU() << ")");
  res.freeResult();
} // findGroups()
}
