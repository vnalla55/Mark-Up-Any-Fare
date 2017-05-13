//----------------------------------------------------------------------------
//  File:           QueryGetSvcFeesResBkgDesig.cpp
//  Description:    QuerySvcFeesResBkgDesig
//  Created:        11/12/2009
// Authors:
//
//  Updates:
//
//  2009, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetSvcFeesResBkgDesig.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetSvcFeesResBkgDesigSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetSvcFeesResBkgDesig::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetSvcFeesResBkgDesig"));
std::string QueryGetSvcFeesResBkgDesig::_baseSQL;
bool QueryGetSvcFeesResBkgDesig::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSvcFeesResBkgDesig> g_GetSvcFeesResBkgDesig;

const char*
QueryGetSvcFeesResBkgDesig::getQueryName() const
{
  return "GETSVCFEESResBkgDesig";
}

void
QueryGetSvcFeesResBkgDesig::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSvcFeesResBkgDesigSQLStatement<QueryGetSvcFeesResBkgDesig> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSVCFEESResBkgDesig");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetSvcFeesResBkgDesig::findSvcFeesResBkgDesigInfo(
    std::vector<tse::SvcFeesResBkgDesigInfo*>& resBkgDesig, const VendorCode& vendor, int itemNo)
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
    resBkgDesig.push_back(QueryGetSvcFeesResBkgDesigSQLStatement<
        QueryGetSvcFeesResBkgDesig>::mapRowToSvcFeesResBkgDesigInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETSVCFEESResBkgDesig: NumRows " << res.numRows() << " Time = " << stopTimer()
                                                 << " (" << stopCPU() << ") mSecs");
  res.freeResult();
} // findSvcFeesResBkgDesigInfo()

///////////////////////////////////////////////////////////
//  QueryGetSvcFeesResBkgDesigHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetSvcFeesResBkgDesigHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetSvcFeesResBkgDesigHistorical"));
std::string QueryGetSvcFeesResBkgDesigHistorical::_baseSQL;
bool QueryGetSvcFeesResBkgDesigHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSvcFeesResBkgDesigHistorical> g_GetSvcFeesResBkgDesigHistorical;

const char*
QueryGetSvcFeesResBkgDesigHistorical::getQueryName() const
{
  return "GETSVCFEESResBkgDesigHISTORICAL";
}

void
QueryGetSvcFeesResBkgDesigHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSvcFeesResBkgDesigHistoricalSQLStatement<QueryGetSvcFeesResBkgDesigHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSVCFEESResBkgDesigHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetSvcFeesResBkgDesigHistorical::findSvcFeesResBkgDesigInfo(
    std::vector<tse::SvcFeesResBkgDesigInfo*>& resBkgDesig,
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
    resBkgDesig.push_back(QueryGetSvcFeesResBkgDesigHistoricalSQLStatement<
        QueryGetSvcFeesResBkgDesigHistorical>::mapRowToSvcFeesResBkgDesigInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETSVCFEESResBkgDesigHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findSvcFeesResBkgDesigInfo()
}
