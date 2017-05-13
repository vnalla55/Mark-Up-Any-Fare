//----------------------------------------------------------------------------
//  File:           QueryGetIndFareBasisMod.cpp
//  Description:    QueryGetIndFareBasisMod
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

#include "DBAccess/Queries/QueryGetIndFareBasisMod.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetIndFareBasisModSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetIndFareBasisMod::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetIndFareBasisMod"));
std::string QueryGetIndFareBasisMod::_baseSQL;
bool QueryGetIndFareBasisMod::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetIndFareBasisMod> g_GetIndFareBasisMod;

const char*
QueryGetIndFareBasisMod::getQueryName() const
{
  return "GETINDFAREBASISMOD";
}

void
QueryGetIndFareBasisMod::initialize()
{
  if (!_isInitialized)
  {
    QueryGetIndFareBasisModSQLStatement<QueryGetIndFareBasisMod> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETINDFAREBASISMOD");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetIndFareBasisMod::findIndustryFareBasisMod(std::vector<tse::IndustryFareBasisMod*>& indFares,
                                                  const CarrierCode& carrier,
                                                  const Indicator& userApplType,
                                                  const UserApplCode& userAppl)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, carrier);
  substParm(2, userApplType);
  substParm(3, userAppl);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    indFares.push_back(
        QueryGetIndFareBasisModSQLStatement<QueryGetIndFareBasisMod>::mapRowToIndustryFareBasisMod(
            row));
  }
  LOG4CXX_INFO(_logger,
               "GETINDFAREBASISMOD: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                << " (" << stopCPU() << ")");
  res.freeResult();
} // findIndustryFareBasisMod()

///////////////////////////////////////////////////////////
//
//  QueryGetIndFareBasisModHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetIndFareBasisModHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetIndFareBasisModHistorical"));
std::string QueryGetIndFareBasisModHistorical::_baseSQL;
bool QueryGetIndFareBasisModHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetIndFareBasisModHistorical> g_GetIndFareBasisModHistorical;

const char*
QueryGetIndFareBasisModHistorical::getQueryName() const
{
  return "GETINDFAREBASISMODHISTORICAL";
}

void
QueryGetIndFareBasisModHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetIndFareBasisModHistoricalSQLStatement<QueryGetIndFareBasisModHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETINDFAREBASISMODHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetIndFareBasisModHistorical::findIndustryFareBasisMod(
    std::vector<tse::IndustryFareBasisMod*>& indFares,
    const CarrierCode& carrier,
    const Indicator& userApplType,
    const UserApplCode& userAppl)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, carrier);
  substParm(2, userApplType);
  substParm(3, userAppl);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    indFares.push_back(QueryGetIndFareBasisModHistoricalSQLStatement<
        QueryGetIndFareBasisModHistorical>::mapRowToIndustryFareBasisMod(row));
  }
  LOG4CXX_INFO(_logger,
               "GETINDFAREBASISMODHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findIndustryFareBasisMod()

///////////////////////////////////////////////////////////
//
//  QueryGetAllIndFareBasisMod
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllIndFareBasisMod::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllIndFareBasisMod"));
std::string QueryGetAllIndFareBasisMod::_baseSQL;
bool QueryGetAllIndFareBasisMod::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllIndFareBasisMod> g_GetAllIndFareBasisMod;

const char*
QueryGetAllIndFareBasisMod::getQueryName() const
{
  return "GETALLINDFAREBASISMOD";
}

void
QueryGetAllIndFareBasisMod::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllIndFareBasisModSQLStatement<QueryGetAllIndFareBasisMod> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLINDFAREBASISMOD");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllIndFareBasisMod::findAllIndustryFareBasisMod(
    std::vector<tse::IndustryFareBasisMod*>& indFares)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    indFares.push_back(QueryGetAllIndFareBasisModSQLStatement<
        QueryGetAllIndFareBasisMod>::mapRowToIndustryFareBasisMod(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLINDFAREBASISMOD: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                   << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllIndustryFareBasisMod()
///////////////////////////////////////////////////////////
//
//  QueryGetAllIndFareBasisModHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllIndFareBasisModHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllIndFareBasisModHistorical"));
std::string QueryGetAllIndFareBasisModHistorical::_baseSQL;
bool QueryGetAllIndFareBasisModHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllIndFareBasisModHistorical> g_GetAllIndFareBasisModHistorical;

const char*
QueryGetAllIndFareBasisModHistorical::getQueryName() const
{
  return "GETALLINDFAREBASISMODHISTORICAL";
}

void
QueryGetAllIndFareBasisModHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllIndFareBasisModHistoricalSQLStatement<QueryGetAllIndFareBasisModHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLINDFAREBASISMODHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllIndFareBasisModHistorical::findAllIndustryFareBasisMod(
    std::vector<tse::IndustryFareBasisMod*>& indFares)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    indFares.push_back(QueryGetAllIndFareBasisModHistoricalSQLStatement<
        QueryGetAllIndFareBasisModHistorical>::mapRowToIndustryFareBasisMod(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLINDFAREBASISMODHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllIndustryFareBasisMod()
}
