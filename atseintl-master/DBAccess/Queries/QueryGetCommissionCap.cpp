//----------------------------------------------------------------------------
//  File:           QueryGetCommissionCap.cpp
//  Description:    QueryGetCommissionCap
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

#include "DBAccess/Queries/QueryGetCommissionCap.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCommissionCapSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetCommissionCap::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetCommissionCap"));
std::string QueryGetCommissionCap::_baseSQL;
bool QueryGetCommissionCap::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCommissionCap> g_GetCommissionCap;

const char*
QueryGetCommissionCap::getQueryName() const
{
  return "GETCOMMISSIONCAP";
}

void
QueryGetCommissionCap::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCommissionCapSQLStatement<QueryGetCommissionCap> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCOMMISSIONCAP");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetCommissionCap::findCommissionCap(std::vector<tse::CommissionCap*>& lstCC,
                                         const CarrierCode& carrier,
                                         const CurrencyCode& cur)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, carrier);
  substParm(2, cur);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()) != nullptr)
  {
    lstCC.push_back(
        QueryGetCommissionCapSQLStatement<QueryGetCommissionCap>::mapRowToCommissionCap(row));
  }
  LOG4CXX_INFO(_logger,
               "GETCOMMISSIONCAP: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                              << stopCPU() << ") mSecs");
  res.freeResult();
} // findCommissionCap()
///////////////////////////////////////////////////////////
//
//  QueryGetCommissionCapHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetCommissionCapHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetCommissionCapHistorical"));
std::string QueryGetCommissionCapHistorical::_baseSQL;
bool QueryGetCommissionCapHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCommissionCapHistorical> g_GetCommissionCapHistorical;

const char*
QueryGetCommissionCapHistorical::getQueryName() const
{
  return "GETCOMMISSIONCAPHISTORICAL";
}

void
QueryGetCommissionCapHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCommissionCapHistoricalSQLStatement<QueryGetCommissionCapHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCOMMISSIONCAPHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetCommissionCapHistorical::findCommissionCap(std::vector<tse::CommissionCap*>& lstCC,
                                                   const CarrierCode& carrier,
                                                   const CurrencyCode& cur)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, carrier);
  substParm(2, cur);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()) != nullptr)
  {
    lstCC.push_back(QueryGetCommissionCapHistoricalSQLStatement<
        QueryGetCommissionCapHistorical>::mapRowToCommissionCap(row));
  }
  LOG4CXX_INFO(_logger,
               "GETCOMMISSIONCAPHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ") mSecs");
  res.freeResult();
} // findCommissionCap()

///////////////////////////////////////////////////////////
//
//  QueryGetAllCommissionCap
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllCommissionCap::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllCommissionCap"));
std::string QueryGetAllCommissionCap::_baseSQL;
bool QueryGetAllCommissionCap::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllCommissionCap> g_GetAllCommissionCap;

const char*
QueryGetAllCommissionCap::getQueryName() const
{
  return "GETALLCOMMISSIONCAP";
}

void
QueryGetAllCommissionCap::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllCommissionCapSQLStatement<QueryGetAllCommissionCap> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLCOMMISSIONCAP");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllCommissionCap::findAllCommissionCap(std::vector<tse::CommissionCap*>& lstCC)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()) != nullptr)
  {
    lstCC.push_back(
        QueryGetAllCommissionCapSQLStatement<QueryGetAllCommissionCap>::mapRowToCommissionCap(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLCOMMISSIONCAP: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                 << " (" << stopCPU() << ") mSecs");
  res.freeResult();
} // findAllCommissionCap()
///////////////////////////////////////////////////////////
//
//  QueryGetAllCommissionCapHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllCommissionCapHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllCommissionCapHistorical"));
std::string QueryGetAllCommissionCapHistorical::_baseSQL;
bool QueryGetAllCommissionCapHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllCommissionCapHistorical> g_GetAllCommissionCapHistorical;

const char*
QueryGetAllCommissionCapHistorical::getQueryName() const
{
  return "GETALLCOMMISSIONCAPHISTORICAL";
}

void
QueryGetAllCommissionCapHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllCommissionCapHistoricalSQLStatement<QueryGetAllCommissionCapHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLCOMMISSIONCAPHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllCommissionCapHistorical::findAllCommissionCap(std::vector<tse::CommissionCap*>& lstCC)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()) != nullptr)
  {
    lstCC.push_back(QueryGetAllCommissionCapHistoricalSQLStatement<
        QueryGetAllCommissionCapHistorical>::mapRowToCommissionCap(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLCOMMISSIONCAPHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ") mSecs");
  res.freeResult();
} // findAllCommissionCap()
}
