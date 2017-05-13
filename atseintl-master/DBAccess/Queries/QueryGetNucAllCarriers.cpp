//----------------------------------------------------------------------------
//  File:           QueryGetNucAllCarriers.cpp
//  Description:    QueryGetNucAllCarriers
//  Created:        11/08/2007
// Authors:         Tomasz Karczewski
//
//  Updates:
//
// � 2007, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetNucAllCarriers.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetNucSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetOneNucAllCarriers::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetOneNucAllCarriers"));
std::string QueryGetOneNucAllCarriers::_baseSQL;
bool QueryGetOneNucAllCarriers::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetOneNucAllCarriers> g_GetOneNucAllCarriers;

const char*
QueryGetOneNucAllCarriers::getQueryName() const
{
  return "GETONENUCALLCARRIERSALLCARRIERS";
}

void
QueryGetOneNucAllCarriers::initialize()
{
  if (!_isInitialized)
  {
    QueryGetOneNucAllCarriersSQLStatement<QueryGetOneNucAllCarriers> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETONENUCALLCARRIERS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetOneNucAllCarriers::findNUC(std::vector<tse::NUCInfo*>& nucs, CurrencyCode cur)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(cur, 1);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    nucs.push_back(
        QueryGetOneNucAllCarriersSQLStatement<QueryGetOneNucAllCarriers>::mapRowToNUCInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETONENUCALLCARRIERS: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                  << " (" << stopCPU() << ")");
  res.freeResult();
} // findNUC()

///////////////////////////////////////////////////////////
//
//  QueryGetAllNucsAllCarriers
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllNucsAllCarriers::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetAllNucsAllCarriers"));
std::string QueryGetAllNucsAllCarriers::_baseSQL;
bool QueryGetAllNucsAllCarriers::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllNucsAllCarriers> g_GetAllNucsAllCarriers;

const char*
QueryGetAllNucsAllCarriers::getQueryName() const
{
  return "GETALLNUCSALLCARRIERS";
}

void
QueryGetAllNucsAllCarriers::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllNucsAllCarriersSQLStatement<QueryGetAllNucsAllCarriers> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLNUCSALLCARRIERS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
tse::QueryGetAllNucsAllCarriers::findAllNUC(std::vector<tse::NUCInfo*>& nucs)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());

  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    nucs.push_back(
        QueryGetAllNucsAllCarriersSQLStatement<QueryGetAllNucsAllCarriers>::mapRowToNUCInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLNUCSALLCARRIERS: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                   << " (" << stopCPU() << ")");

  res.freeResult();
} // findAllNUC()

///////////////////////////////////////////////////////////
//
//  QueryGetNucAllCarriersHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetNucAllCarriersHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetNucAllCarriersHistorical"));
std::string QueryGetNucAllCarriersHistorical::_baseSQL;
bool QueryGetNucAllCarriersHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetNucAllCarriersHistorical> g_GetNucAllCarriersHistorical;

const char*
QueryGetNucAllCarriersHistorical::getQueryName() const
{
  return "GETNUCALLCARRIERSHISTORICAL";
}

void
QueryGetNucAllCarriersHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetNucAllCarriersHistoricalSQLStatement<QueryGetNucAllCarriersHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETNUCALLCARRIERSHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetNucAllCarriersHistorical::findNUC(std::vector<tse::NUCInfo*>& nucs, CurrencyCode cur)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(cur, 1);
  substParm(cur, 2);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    nucs.push_back(QueryGetNucAllCarriersHistoricalSQLStatement<
        QueryGetNucAllCarriersHistorical>::mapRowToNUCInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETNUCALLCARRIERSHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetNucAllCarriersHistorical::findNUC()
}
