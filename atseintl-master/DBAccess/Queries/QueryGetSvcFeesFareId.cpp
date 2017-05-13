//----------------------------------------------------------------------------
//  File:           QueryGetSvcFeesFareId.cpp
//  Description:    QueryGetSvcFeesFareId
//  Created:        04/08/2013
//
// Copyright 2013, Sabre Inc. All rights reserved. This software/documentation is the
// confidential and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetSvcFeesFareId.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetSvcFeesFareIdSQLStatement.h"

namespace tse
{

log4cxx::LoggerPtr
QueryGetSvcFeesFareId::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetSvcFeesFareId"));
std::string QueryGetSvcFeesFareId::_baseSQL;
bool
QueryGetSvcFeesFareId::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetSvcFeesFareId> _getSvcFeesFareId;

const char*
QueryGetSvcFeesFareId::getQueryName() const
{
  return "GETSVCFEESFAREID";
}

void
QueryGetSvcFeesFareId::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSvcFeesFareIdSQLStatement<QueryGetSvcFeesFareId> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSVCFEESFAREID");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetSvcFeesFareId::findSvcFeesFareId(std::vector<SvcFeesFareIdInfo*>& lst,
                                         const VendorCode& vendor,
                                         long long itemNo)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);
  substParm(vendor, 1);
  char itemStr[32] = {};
  sprintf(itemStr, "%lld", itemNo);
  substParm(itemStr, 2);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    SvcFeesFareIdInfo* info(
        QueryGetSvcFeesFareIdSQLStatement<QueryGetSvcFeesFareId>::mapRowToSvcFeesFareId(row));
    lst.push_back(info);
  }
  LOG4CXX_INFO(_logger,
               "GETSVCFEESFAREID: NumRows: " << res.numRows() << " Time = " << stopTimer() << " ("
                                             << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetSvcFeesFareIdHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetSvcFeesFareIdHistorical"));
std::string QueryGetSvcFeesFareIdHistorical::_baseSQL;
bool
QueryGetSvcFeesFareIdHistorical::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetSvcFeesFareIdHistorical> _getSvcFeesFareIdHistorical;

const char*
QueryGetSvcFeesFareIdHistorical::getQueryName() const
{
  return "GETSVCFEESFAREIDHISTORICAL";
}

void
QueryGetSvcFeesFareIdHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSvcFeesFareIdHistoricalSQLStatement<QueryGetSvcFeesFareIdHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSVCFEESFAREIDHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetSvcFeesFareIdHistorical::findSvcFeesFareId(std::vector<SvcFeesFareIdInfo*>& lst,
                                                   const VendorCode& vendor,
                                                   long long itemNo,
                                                   const DateTime& startDate,
                                                   const DateTime& endDate)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  substParm(vendor, 1);
  char itemStr[32] = {};
  sprintf(itemStr, "%lld", itemNo);
  substParm(itemStr, 2);
  substParm(3, startDate);
  substParm(4, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    SvcFeesFareIdInfo* info(QueryGetSvcFeesFareIdHistoricalSQLStatement<
        QueryGetSvcFeesFareIdHistorical>::mapRowToSvcFeesFareId(row));
    lst.push_back(info);
  }
  LOG4CXX_INFO(_logger,
               "GETSVCFEESFAREIDHISTORICAL: NumRows: " << res.numRows() << " Time = " << stopTimer()
                                                       << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetAllSvcFeesFareId::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllSvcFeesFareId"));
std::string QueryGetAllSvcFeesFareId::_baseSQL;
bool
QueryGetAllSvcFeesFareId::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetAllSvcFeesFareId> _getAllSvcFeesFareId;

const char*
QueryGetAllSvcFeesFareId::getQueryName() const
{
  return "GETALLSVCFEESFAREID";
}

void
QueryGetAllSvcFeesFareId::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllSvcFeesFareIdSQLStatement<QueryGetAllSvcFeesFareId> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLSVCFEESFAREID");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetAllSvcFeesFareId::findAllSvcFeesFareId(std::vector<SvcFeesFareIdInfo*>& lst)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    SvcFeesFareIdInfo* info(
        QueryGetAllSvcFeesFareIdSQLStatement<QueryGetAllSvcFeesFareId>::mapRowToSvcFeesFareId(row));
    lst.push_back(info);
  }
  LOG4CXX_INFO(_logger,
               "GETALLSVCFEESFAREID: NumRows: " << res.numRows() << " Time = " << stopTimer()
                                                << " (" << stopCPU() << ") ms");
  res.freeResult();
}
} // tse
