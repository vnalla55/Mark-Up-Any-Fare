//----------------------------------------------------------------------------
//  File:           QueryGetAllServiceGroup.cpp
//  Description:    QueryGetAllServiceGroup
//  Created:        2/8/2010
// Authors:
//
//  Updates:
//
//  2010, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetServiceGroup.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetServiceGroupSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetAllServiceGroup::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetAllServiceGroup"));
std::string QueryGetAllServiceGroup::_baseSQL;
bool QueryGetAllServiceGroup::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllServiceGroup> g_GetAllServiceGroup;

const char*
QueryGetAllServiceGroup::getQueryName() const
{
  return "GETALLSERVICEGROUP";
}

void
QueryGetAllServiceGroup::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllServiceGroupSQLStatement<QueryGetAllServiceGroup> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLSERVICEGROUP");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllServiceGroup::findAllServiceGroup(std::vector<tse::ServiceGroupInfo*>& groups)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    groups.push_back(
        QueryGetAllServiceGroupSQLStatement<QueryGetAllServiceGroup>::mapRowToServiceGroup(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLSERVICEGROUP: NumRows " << res.numRows() << " Time = " << stopTimer() << " ("
                                              << stopCPU() << ") mSecs");
  res.freeResult();
} // findAllServiceGroup()

///////////////////////////////////////////////////////////
//  QueryGetServiceGroupHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllServiceGroupHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetServiceGroupHistorical"));
std::string QueryGetAllServiceGroupHistorical::_baseSQL;
bool QueryGetAllServiceGroupHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllServiceGroupHistorical> g_GetAllServiceGroupHistorical;

const char*
QueryGetAllServiceGroupHistorical::getQueryName() const
{
  return "GETALLSERVICEGROUPHISTORICAL";
}

void
QueryGetAllServiceGroupHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllServiceGroupHistoricalSQLStatement<QueryGetAllServiceGroupHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLSERVICEGROUPHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllServiceGroupHistorical::findAllServiceGroup(std::vector<tse::ServiceGroupInfo*>& groups)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    groups.push_back(QueryGetAllServiceGroupHistoricalSQLStatement<
        QueryGetAllServiceGroupHistorical>::mapRowToServiceGroup(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLSERVICEGROUPHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findServiceGroup()
}
