//----------------------------------------------------------------------------
//  File:           QueryGetJointCxr.cpp
//  Description:    QueryGetJointCxr
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
#include "DBAccess/Queries/QueryGetJointCxr.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetJointCxrSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetJointCxr::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetJointCxr"));
std::string QueryGetJointCxr::_baseSQL;
bool QueryGetJointCxr::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetJointCxr> g_GetJointCxr;

const char*
QueryGetJointCxr::getQueryName() const
{
  return "GETJOINTCXR";
}

void
QueryGetJointCxr::initialize()
{
  if (!_isInitialized)
  {
    QueryGetJointCxrSQLStatement<QueryGetJointCxr> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETJOINTCXR");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetJointCxr::findJointCarrier(std::vector<const tse::JointCarrier*>& jointcxrs,
                                   const VendorCode& vendor,
                                   int itemNo)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char itemStr[15];
  sprintf(itemStr, "%d", itemNo);

  substParm(1, vendor);
  substParm(2, itemNo);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    jointcxrs.push_back(QueryGetJointCxrSQLStatement<QueryGetJointCxr>::mapRowToJointCarrier(row));
  }
  LOG4CXX_INFO(_logger,
               "GETJOINTCXR: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                         << stopCPU() << ")");
  res.freeResult();
} // findJointCarrier()

///////////////////////////////////////////////////////////
//  QueryGetJointCxrHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetJointCxrHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetJointCxrHistorical"));
std::string QueryGetJointCxrHistorical::_baseSQL;
bool QueryGetJointCxrHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetJointCxrHistorical> g_GetJointCxrHistorical;

const char*
QueryGetJointCxrHistorical::getQueryName() const
{
  return "GETJOINTCXRHISTORICAL";
}

void
QueryGetJointCxrHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetJointCxrHistoricalSQLStatement<QueryGetJointCxrHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETJOINTCXRHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetJointCxrHistorical::findJointCarrier(std::vector<const tse::JointCarrier*>& jointcxrs,
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
  substParm(2, itemNo);
  substParm(3, startDate);
  substParm(4, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    jointcxrs.push_back(
        QueryGetJointCxrHistoricalSQLStatement<QueryGetJointCxrHistorical>::mapRowToJointCarrier(
            row));
  }
  LOG4CXX_INFO(_logger,
               "GETJOINTCXRHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                   << " (" << stopCPU() << ")");
  res.freeResult();
} // findJointCarrier()
}
