//----------------------------------------------------------------------------
//          File:           QueryGetTPMExclusion.h
//          Description:    QueryGetTPMExclusion
//          Created:        08/10/2009
//          Authors:        Adam Szalajko
//
//          Updates:
//
//     � 2007, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetTPMExclusion.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetTPMExclusionSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetTPMExclusion::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetTPMExclusion"));
std::string QueryGetTPMExclusion::_baseSQL;
bool QueryGetTPMExclusion::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTPMExclusion> g_GetTPMExclusion;

const char*
QueryGetTPMExclusion::getQueryName() const
{
  return "QUERYGETTPMEXCLUSION";
}

void
QueryGetTPMExclusion::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTPMExclusionSQLStatement<QueryGetTPMExclusion> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("QUERYGETTPMEXCLUSION");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTPMExclusion::findTPMExcl(std::vector<tse::TPMExclusion*>& tpms, const CarrierCode& carrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  substParm(1, carrier);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    tpms.push_back(QueryGetTPMExclusionSQLStatement<QueryGetTPMExclusion>::mapRowToTPMExcl(row));
  }
  LOG4CXX_INFO(_logger,
               "QUERYGETTPMEXCLUSION: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                  << " (" << stopCPU() << ")");
  res.freeResult();
}

///////////////////////////////////////////////////////////
//
//  QueryGetTPMExclusionHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetTPMExclusionHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetTPMExclusionHistorical"));
std::string QueryGetTPMExclusionHistorical::_baseSQL;
bool QueryGetTPMExclusionHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTPMExclusionHistorical> g_GetTPMExclusionHistorical;

const char*
QueryGetTPMExclusionHistorical::getQueryName() const
{
  return "QUERYGETTPMEXCLUSIONHISTORICAL";
}

void
QueryGetTPMExclusionHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTPMExclusionHistoricalSQLStatement<QueryGetTPMExclusionHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("QUERYGETTPMEXCLUSIONHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTPMExclusionHistorical::findTPMExcl(std::vector<tse::TPMExclusion*>& tpms,
                                            const CarrierCode& carrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, carrier);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    tpms.push_back(
        QueryGetTPMExclusionHistoricalSQLStatement<QueryGetTPMExclusionHistorical>::mapRowToTPMExcl(
            row));
  }
  LOG4CXX_INFO(_logger,
               "QUERYGETTPMEXCLUSIONHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
}

///////////////////////////////////////////////////////////
//
//  QueryGetAllTPMExclusion()
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllTPMExclusion::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetAllTPMExclusion"));
std::string QueryGetAllTPMExclusion::_baseSQL;
bool QueryGetAllTPMExclusion::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllTPMExclusion> g_GetAllTPMExclusion;

const char*
QueryGetAllTPMExclusion::getQueryName() const
{
  return "QUERYGETALLTPMEXCLUSION";
}
void
QueryGetAllTPMExclusion::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllTPMExclusionSQLStatement<QueryGetAllTPMExclusion> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("QUERYGETALLTPMEXCLUSION");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllTPMExclusion::findAllTPMExcl(std::vector<tse::TPMExclusion*>& tpms)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    tpms.push_back(
        QueryGetAllTPMExclusionSQLStatement<QueryGetAllTPMExclusion>::mapRowToTPMExcl(row));
  }
  LOG4CXX_INFO(_logger,
               "QUERYGETALLTPMEXCLUSION: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                     << " (" << stopCPU() << ")");
  res.freeResult();
}

///////////////////////////////////////////////////////////
//
//  QueryGetAllTPMExclusionHistorical()
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllTPMExclusionHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetAllTPMExclusionHistorical"));
std::string QueryGetAllTPMExclusionHistorical::_baseSQL;
bool QueryGetAllTPMExclusionHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllTPMExclusionHistorical> g_GetAllTPMExclusionHistorical;

const char*
QueryGetAllTPMExclusionHistorical::getQueryName() const
{
  return "QUERYGETALLTPMEXCLUSIONHISTORICAL";
}

void
QueryGetAllTPMExclusionHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllTPMExclusionHistoricalSQLStatement<QueryGetAllTPMExclusionHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("QUERYGETALLTPMEXCLUSIONHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetAllTPMExclusionHistorical::findAllTPMExcl(std::vector<tse::TPMExclusion*>& tpms)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    tpms.push_back(QueryGetAllTPMExclusionHistoricalSQLStatement<
        QueryGetAllTPMExclusionHistorical>::mapRowToTPMExcl(row));
  }
  LOG4CXX_INFO(_logger,
               "QUERYGETALLTPMEXCLUSIONHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
}
}
