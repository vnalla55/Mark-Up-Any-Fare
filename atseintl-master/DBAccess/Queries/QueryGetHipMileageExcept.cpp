//----------------------------------------------------------------------------
//  File:           QueryGetHipMileageExcept.cpp
//  Description:    QueryGetHipMileageExcept
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
#include "DBAccess/Queries/QueryGetHipMileageExcept.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetHipMileageExceptSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetHipMileageExcept::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetHipMileageExcept"));
std::string QueryGetHipMileageExcept::_baseSQL;
bool QueryGetHipMileageExcept::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetHipMileageExcept> g_GetHipMileageExcept;

const char*
QueryGetHipMileageExcept::getQueryName() const
{
  return "GETHIPMILEAGEEXCEPT";
}

void
QueryGetHipMileageExcept::initialize()
{
  if (!_isInitialized)
  {
    QueryGetHipMileageExceptSQLStatement<QueryGetHipMileageExcept> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETHIPMILEAGEEXCEPT");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetHipMileageExcept::findHipMileageExcept(
    std::vector<const tse::HipMileageExceptInfo*>& lstHME, VendorCode& vendor, int itemNo)
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
  while ((row = res.nextRow()) != nullptr)
  {
    lstHME.push_back(QueryGetHipMileageExceptSQLStatement<
        QueryGetHipMileageExcept>::mapRowToHipMileageExceptInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETHIPMILEAGEEXCEPT: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                 << " (" << stopCPU() << ") mSecs");
  res.freeResult();
} // findHipMileageExcept()

///////////////////////////////////////////////////////////
//  QueryGetHipMileageExceptHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetHipMileageExceptHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetHipMileageExceptHistorical"));
std::string QueryGetHipMileageExceptHistorical::_baseSQL;
bool QueryGetHipMileageExceptHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetHipMileageExceptHistorical> g_GetHipMileageExceptHistorical;

const char*
QueryGetHipMileageExceptHistorical::getQueryName() const
{
  return "GETHIPMILEAGEEXCEPTHISTORICAL";
}

void
QueryGetHipMileageExceptHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetHipMileageExceptHistoricalSQLStatement<QueryGetHipMileageExceptHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETHIPMILEAGEEXCEPTHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetHipMileageExceptHistorical::findHipMileageExcept(
    std::vector<const tse::HipMileageExceptInfo*>& lstHME,
    VendorCode& vendor,
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
    lstHME.push_back(QueryGetHipMileageExceptHistoricalSQLStatement<
        QueryGetHipMileageExceptHistorical>::mapRowToHipMileageExceptInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETHIPMILEAGEEXCEPTHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findHipMileageExcept()
}
