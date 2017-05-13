//----------------------------------------------------------------------------
//  File:           QueryGetIndPriceAppl.cpp
//  Description:    QueryGetIndPriceAppl
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

#include "DBAccess/Queries/QueryGetIndPriceAppl.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetIndPriceApplSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetIndPriceAppl::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetIndPriceAppl"));
std::string QueryGetIndPriceAppl::_baseSQL;
bool QueryGetIndPriceAppl::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetIndPriceAppl> g_GetIndPriceAppl;

const char*
QueryGetIndPriceAppl::getQueryName() const
{
  return "GETINDPRICEAPPL";
}

void
QueryGetIndPriceAppl::initialize()
{
  if (!_isInitialized)
  {
    QueryGetIndPriceApplSQLStatement<QueryGetIndPriceAppl> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETINDPRICEAPPL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetIndPriceAppl::findIndustryPricingAppl(
    std::vector<const tse::IndustryPricingAppl*>& indPriceAppls, const CarrierCode& carrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, carrier);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    indPriceAppls.push_back(
        QueryGetIndPriceApplSQLStatement<QueryGetIndPriceAppl>::mapRowToIndustryPricingAppl(row));
  }
  LOG4CXX_INFO(_logger,
               "GETINDPRICEAPPL: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                             << stopCPU() << ")");
  res.freeResult();
} // findIndustryPricingAppl()

///////////////////////////////////////////////////////////
//
//  QueryGetAllIndPriceAppl
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllIndPriceAppl::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllIndPriceAppl"));
std::string QueryGetAllIndPriceAppl::_baseSQL;
bool QueryGetAllIndPriceAppl::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllIndPriceAppl> g_GetAllIndPriceAppl;

const char*
QueryGetAllIndPriceAppl::getQueryName() const
{
  return "GETALLINDPRICEAPPL";
}

void
QueryGetAllIndPriceAppl::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllIndPriceApplSQLStatement<QueryGetAllIndPriceAppl> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLINDPRICEAPPL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllIndPriceAppl::findAllIndustryPricingAppl(
    std::vector<const tse::IndustryPricingAppl*>& indPriceAppls)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    indPriceAppls.push_back(
        QueryGetAllIndPriceApplSQLStatement<QueryGetAllIndPriceAppl>::mapRowToIndustryPricingAppl(
            row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLINDPRICEAPPL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllIndustryPricingAppl()

///////////////////////////////////////////////////////////
//
//  QueryGetIndPriceApplHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetIndPriceApplHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetIndPriceApplHistorical"));
std::string QueryGetIndPriceApplHistorical::_baseSQL;
bool QueryGetIndPriceApplHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetIndPriceApplHistorical> g_GetIndPriceApplHistorical;

const char*
QueryGetIndPriceApplHistorical::getQueryName() const
{
  return "GETINDPRICEAPPLHISTORICAL";
}

void
QueryGetIndPriceApplHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetIndPriceApplHistoricalSQLStatement<QueryGetIndPriceApplHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETINDPRICEAPPLHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetIndPriceApplHistorical::findIndustryPricingAppl(
    std::vector<tse::IndustryPricingAppl*>& indPriceAppls, const CarrierCode& carrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, carrier);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    indPriceAppls.push_back(QueryGetIndPriceApplHistoricalSQLStatement<
        QueryGetIndPriceApplHistorical>::mapRowToIndustryPricingAppl(row));
  }
  LOG4CXX_INFO(_logger,
               "GETINDPRICEAPPLHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                       << " (" << stopCPU() << ")");
  res.freeResult();
} // findIndustryPricingAppl()

///////////////////////////////////////////////////////////
//
//  QueryGetAllIndPriceApplHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllIndPriceApplHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllIndPriceApplHistorical"));
std::string QueryGetAllIndPriceApplHistorical::_baseSQL;
bool QueryGetAllIndPriceApplHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllIndPriceApplHistorical> g_GetAllIndPriceApplHistorical;

const char*
QueryGetAllIndPriceApplHistorical::getQueryName() const
{
  return "GETALLINDPRICEAPPLHISTORICAL";
}

void
QueryGetAllIndPriceApplHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllIndPriceApplHistoricalSQLStatement<QueryGetAllIndPriceApplHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLINDPRICEAPPLHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllIndPriceApplHistorical::findAllIndustryPricingAppl(
    std::vector<tse::IndustryPricingAppl*>& indPriceAppls)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    indPriceAppls.push_back(QueryGetAllIndPriceApplHistoricalSQLStatement<
        QueryGetAllIndPriceApplHistorical>::mapRowToIndustryPricingAppl(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLINDPRICEAPPLHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllIndustryPricingAppl()
}
