//----------------------------------------------------------------------------
//  File:           QueryGetSvcFeesCxrResultingFCL.cpp
//  Description:    QueryGetSvcFeesCxrResultingFCL
//  Created:        11/10/2009
// Authors:
//
//  Updates:
//
//  2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetSvcFeesCxrResultingFCL.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetSvcFeesCxrResultingFCLSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetSvcFeesCxrResultingFCL::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetSvcFeesCxrResultingFCL"));
std::string QueryGetSvcFeesCxrResultingFCL::_baseSQL;
bool QueryGetSvcFeesCxrResultingFCL::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSvcFeesCxrResultingFCL> g_GetSvcFeesCxrResultingFCL;

const char*
QueryGetSvcFeesCxrResultingFCL::getQueryName() const
{
  return "GETSVCFEESCXRRESULTINGFCL";
}

void
QueryGetSvcFeesCxrResultingFCL::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSvcFeesCxrResultingFCLSQLStatement<QueryGetSvcFeesCxrResultingFCL> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSVCFEESCXRRESULTINGFCL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetSvcFeesCxrResultingFCL::findSvcFeesCxrResultingFCLInfo(
    std::vector<tse::SvcFeesCxrResultingFCLInfo*>& cxrResultingFCL,
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
    cxrResultingFCL.push_back(QueryGetSvcFeesCxrResultingFCLSQLStatement<
        QueryGetSvcFeesCxrResultingFCL>::mapRowToSvcFeesCxrResultingFCLInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETSVCFEESCXRRESULTINGFCL: NumRows " << res.numRows() << " Time = " << stopTimer()
                                                     << " (" << stopCPU() << ") mSecs");
  res.freeResult();
} // findSvcFeesCxrResultingFCLInfo()

///////////////////////////////////////////////////////////
//  QueryGetSvcFeesCxrResultingFCLHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetSvcFeesCxrResultingFCLHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.QueryGetSvcFeesCxrResultingFCLHistorical"));
std::string QueryGetSvcFeesCxrResultingFCLHistorical::_baseSQL;
bool QueryGetSvcFeesCxrResultingFCLHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSvcFeesCxrResultingFCLHistorical>
g_GetSvcFeesCxrResultingFCLHistorical;

const char*
QueryGetSvcFeesCxrResultingFCLHistorical::getQueryName() const
{
  return "GETSVCFEESCXRRESULTINGFCLHISTORICAL";
}

void
QueryGetSvcFeesCxrResultingFCLHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSvcFeesCxrResultingFCLHistoricalSQLStatement<QueryGetSvcFeesCxrResultingFCLHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSVCFEESCXRRESULTINGFCLHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetSvcFeesCxrResultingFCLHistorical::findSvcFeesCxrResultingFCLInfo(
    std::vector<tse::SvcFeesCxrResultingFCLInfo*>& cxrResultingFCL,
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
    cxrResultingFCL.push_back(QueryGetSvcFeesCxrResultingFCLHistoricalSQLStatement<
        QueryGetSvcFeesCxrResultingFCLHistorical>::mapRowToSvcFeesCxrResultingFCLInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETSVCFEESCXRRESULTINGFCLHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findSvcFeesCxrResultingFCLInfo()
}
