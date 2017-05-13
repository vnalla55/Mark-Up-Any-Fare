//----------------------------------------------------------------------------
//  File:           QueryGetSvcFeesFeature.cpp
//  Description:    QueryGetSvcFeesFeature
//  Created:        04/08/2013
//
// Copyright 2013, Sabre Inc. All rights reserved. This software/documentation is the
// confidential and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetSvcFeesFeature.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetSvcFeesFeatureSQLStatement.h"

namespace tse
{

log4cxx::LoggerPtr
QueryGetSvcFeesFeature::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetSvcFeesFeature"));
std::string QueryGetSvcFeesFeature::_baseSQL;
bool
QueryGetSvcFeesFeature::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetSvcFeesFeature> _getSvcFeesFeature;

const char*
QueryGetSvcFeesFeature::getQueryName() const
{
  return "GETSVCFEESFEATURE";
}

void
QueryGetSvcFeesFeature::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSvcFeesFeatureSQLStatement<QueryGetSvcFeesFeature> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSVCFEESFEATURE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetSvcFeesFeature::findSvcFeesFeature(std::vector<SvcFeesFeatureInfo*>& lst,
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
    SvcFeesFeatureInfo* info(
        QueryGetSvcFeesFeatureSQLStatement<QueryGetSvcFeesFeature>::mapRowToSvcFeesFeature(row));
    lst.push_back(info);
  }
  LOG4CXX_INFO(_logger,
               "GETSVCFEESFEATURE: NumRows: " << res.numRows() << " Time = " << stopTimer() << " ("
                                              << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetSvcFeesFeatureHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetSvcFeesFeatureHistorical"));
std::string QueryGetSvcFeesFeatureHistorical::_baseSQL;
bool
QueryGetSvcFeesFeatureHistorical::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetSvcFeesFeatureHistorical> _getSvcFeesFeatureHistorical;

const char*
QueryGetSvcFeesFeatureHistorical::getQueryName() const
{
  return "GETSVCFEESFEATUREHISTORICAL";
}

void
QueryGetSvcFeesFeatureHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSvcFeesFeatureHistoricalSQLStatement<QueryGetSvcFeesFeatureHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSVCFEESFEATUREHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetSvcFeesFeatureHistorical::findSvcFeesFeature(std::vector<SvcFeesFeatureInfo*>& lst,
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
    SvcFeesFeatureInfo* info(QueryGetSvcFeesFeatureHistoricalSQLStatement<
        QueryGetSvcFeesFeatureHistorical>::mapRowToSvcFeesFeature(row));
    lst.push_back(info);
  }
  LOG4CXX_INFO(_logger,
               "GETSVCFEESFEATUREHISTORICAL: NumRows: "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetAllSvcFeesFeature::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllSvcFeesFeature"));
std::string QueryGetAllSvcFeesFeature::_baseSQL;
bool
QueryGetAllSvcFeesFeature::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetAllSvcFeesFeature> _getAllSvcFeesFeature;

const char*
QueryGetAllSvcFeesFeature::getQueryName() const
{
  return "GETALLSVCFEESFEATURE";
}

void
QueryGetAllSvcFeesFeature::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllSvcFeesFeatureSQLStatement<QueryGetAllSvcFeesFeature> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLSVCFEESFEATURE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetAllSvcFeesFeature::findAllSvcFeesFeature(std::vector<SvcFeesFeatureInfo*>& lst)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    SvcFeesFeatureInfo* info(
        QueryGetAllSvcFeesFeatureSQLStatement<QueryGetAllSvcFeesFeature>::mapRowToSvcFeesFeature(
            row));
    lst.push_back(info);
  }
  LOG4CXX_INFO(_logger,
               "GETALLSVCFEESFEATURE: NumRows: " << res.numRows() << " Time = " << stopTimer()
                                                 << " (" << stopCPU() << ") ms");
  res.freeResult();
}
} // tse
