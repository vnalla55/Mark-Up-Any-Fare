//----------------------------------------------------------------------------
//  File:           QueryGetCopParticipatingNation.cpp
//  Description:    QueryGetCopParticipatingNation
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

#include "DBAccess/Queries/QueryGetCopParticipatingNation.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCopParticipatingNationSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetCopParticipatingNation::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetCopParticipatingNation"));
std::string QueryGetCopParticipatingNation::_baseSQL;
bool QueryGetCopParticipatingNation::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCopParticipatingNation> g_GetCopParticipatingNation;

const char*
QueryGetCopParticipatingNation::getQueryName() const
{
  return "GETCOPPARTICIPATINGNATION";
}

void
QueryGetCopParticipatingNation::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCopParticipatingNationSQLStatement<QueryGetCopParticipatingNation> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCOPPARTICIPATINGNATION");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetCopParticipatingNation::findCopParticipatingNation(
    std::vector<tse::CopParticipatingNation*>& lstCPN,
    const NationCode& nation,
    const NationCode& copNation)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, nation);
  substParm(2, copNation);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstCPN.push_back(QueryGetCopParticipatingNationSQLStatement<
        QueryGetCopParticipatingNation>::mapRowToCopParticipatingNation(row));
  }
  LOG4CXX_INFO(_logger,
               "GETCOPPARTICIPATINGNATION: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                       << " mSecs");
  res.freeResult();
} // findCopParticipatingNation()
///////////////////////////////////////////////////////////
//
//  QueryGetCopParticipatingNationHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetCopParticipatingNationHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetCopParticipatingNationHistorical"));
std::string QueryGetCopParticipatingNationHistorical::_baseSQL;
bool QueryGetCopParticipatingNationHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCopParticipatingNationHistorical>
g_GetCopParticipatingNationHistorical;

const char*
QueryGetCopParticipatingNationHistorical::getQueryName() const
{
  return "GETCOPPARTICIPATINGNATIONHISTORICAL";
}

void
QueryGetCopParticipatingNationHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCopParticipatingNationHistoricalSQLStatement<QueryGetCopParticipatingNationHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCOPPARTICIPATINGNATIONHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetCopParticipatingNationHistorical::findCopParticipatingNation(
    std::vector<tse::CopParticipatingNation*>& lstCPN,
    const NationCode& nation,
    const NationCode& copNation)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, nation);
  substParm(2, copNation);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstCPN.push_back(QueryGetCopParticipatingNationHistoricalSQLStatement<
        QueryGetCopParticipatingNationHistorical>::mapRowToCopParticipatingNation(row));
  }
  LOG4CXX_INFO(_logger,
               "GETCOPPARTICIPATINGNATIONHISTORICAL: NumRows = " << res.numRows() << " Time = "
                                                                 << stopTimer() << " mSecs");
  res.freeResult();
} // findCopParticipatingNation()

///////////////////////////////////////////////////////////
//
//  QueryGetAllCopParticipatingNation
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllCopParticipatingNation::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllCopParticipatingNation"));
std::string QueryGetAllCopParticipatingNation::_baseSQL;
bool QueryGetAllCopParticipatingNation::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllCopParticipatingNation> g_GetAllCopParticipatingNation;

const char*
QueryGetAllCopParticipatingNation::getQueryName() const
{
  return "GETALLCOPPARTICIPATINGNATION";
}

void
QueryGetAllCopParticipatingNation::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllCopParticipatingNationSQLStatement<QueryGetAllCopParticipatingNation> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLCOPPARTICIPATINGNATION");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllCopParticipatingNation::findAllCopParticipatingNation(
    std::vector<tse::CopParticipatingNation*>& lstCPN)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstCPN.push_back(QueryGetAllCopParticipatingNationSQLStatement<
        QueryGetAllCopParticipatingNation>::mapRowToCopParticipatingNation(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLCOPPARTICIPATINGNATION: NumRows = " << res.numRows()
                                                          << " Time = " << stopTimer() << " mSecs");
  res.freeResult();
} // findAllCopParticipatingNation()
///////////////////////////////////////////////////////////
//
//  QueryGetAllCopParticipatingNationHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllCopParticipatingNationHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetAllCopParticipatingNationHistorical"));
std::string QueryGetAllCopParticipatingNationHistorical::_baseSQL;
bool QueryGetAllCopParticipatingNationHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllCopParticipatingNationHistorical>
g_GetAllCopParticipatingNationHistorical;

const char*
QueryGetAllCopParticipatingNationHistorical::getQueryName() const
{
  return "GETALLCOPPARTICIPATINGNATIONHISTORICAL";
}

void
QueryGetAllCopParticipatingNationHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllCopParticipatingNationHistoricalSQLStatement<
        QueryGetAllCopParticipatingNationHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLCOPPARTICIPATINGNATIONHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllCopParticipatingNationHistorical::findAllCopParticipatingNation(
    std::vector<tse::CopParticipatingNation*>& lstCPN)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstCPN.push_back(QueryGetAllCopParticipatingNationHistoricalSQLStatement<
        QueryGetAllCopParticipatingNationHistorical>::mapRowToCopParticipatingNation(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLCOPPARTICIPATINGNATIONHISTORICAL: NumRows = " << res.numRows() << " Time = "
                                                                    << stopTimer() << " mSecs");
  res.freeResult();
} // findAllCopParticipatingNation()
}
