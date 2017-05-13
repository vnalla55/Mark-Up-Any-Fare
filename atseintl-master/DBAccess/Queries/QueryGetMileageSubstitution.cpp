//----------------------------------------------------------------------------
//  File:           QueryGetMileageSubstitution.cpp
//  Description:    QueryGetMileageSubstitution
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

#include "DBAccess/Queries/QueryGetMileageSubstitution.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetMileageSubstitutionSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetMileageSubstitution::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetMileageSubstitution"));
std::string QueryGetMileageSubstitution::_baseSQL;
bool QueryGetMileageSubstitution::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMileageSubstitution> g_GetMileageSubstitution;

const char*
QueryGetMileageSubstitution::getQueryName() const
{
  return "GETMILEAGESUBSTITUTION";
}

void
QueryGetMileageSubstitution::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMileageSubstitutionSQLStatement<QueryGetMileageSubstitution> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMILEAGESUBSTITUTION");

    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMileageSubstitution::findMileageSubstitution(std::vector<tse::MileageSubstitution*>& lstMS,
                                                     const LocCode& substitutionLoc)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, substitutionLoc);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstMS.push_back(QueryGetMileageSubstitutionSQLStatement<
        QueryGetMileageSubstitution>::mapRowToMileageSubstitution(row));
  }
  LOG4CXX_INFO(_logger,
               "GETMILEAGESUBSTITUTION: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                    << " (" << stopCPU() << ")");
  res.freeResult();
} // findMileageSubstitution()
///////////////////////////////////////////////////////////
//
//  QueryGetMileageSubstitutionHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetMileageSubstitutionHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetMileageSubstitutionHistorical"));
std::string QueryGetMileageSubstitutionHistorical::_baseSQL;
bool QueryGetMileageSubstitutionHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMileageSubstitutionHistorical> g_GetMileageSubstitutionHistorical;

const char*
QueryGetMileageSubstitutionHistorical::getQueryName() const
{
  return "GETMILEAGESUBSTITUTIONHISTORICAL";
}

void
QueryGetMileageSubstitutionHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMileageSubstitutionHistoricalSQLStatement<QueryGetMileageSubstitutionHistorical>
    sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMILEAGESUBSTITUTIONHISTORICAL");

    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMileageSubstitutionHistorical::findMileageSubstitution(
    std::vector<tse::MileageSubstitution*>& lstMS, const LocCode& substitutionLoc)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, substitutionLoc);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstMS.push_back(QueryGetMileageSubstitutionHistoricalSQLStatement<
        QueryGetMileageSubstitutionHistorical>::mapRowToMileageSubstitution(row));
  }
  LOG4CXX_INFO(_logger,
               "GETMILEAGESUBSTITUTIONHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findMileageSubstitution()

///////////////////////////////////////////////////////////
//
//  QueryGetAllMileageSubstitution
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllMileageSubstitution::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllMileageSubstitution"));
std::string QueryGetAllMileageSubstitution::_baseSQL;
bool QueryGetAllMileageSubstitution::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllMileageSubstitution> g_GetAllMileageSubstitution;

const char*
QueryGetAllMileageSubstitution::getQueryName() const
{
  return "GETALLMILEAGESUBSTITUTION";
}

void
QueryGetAllMileageSubstitution::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllMileageSubstitutionSQLStatement<QueryGetAllMileageSubstitution> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLMILEAGESUBSTITUTION");

    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllMileageSubstitution::findAllMileageSubstitution(
    std::vector<tse::MileageSubstitution*>& lstMS)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstMS.push_back(QueryGetAllMileageSubstitutionSQLStatement<
        QueryGetAllMileageSubstitution>::mapRowToMileageSubstitution(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLMILEAGESUBSTITUTION: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                       << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllMileageSubstitution()
///////////////////////////////////////////////////////////
//
//  QueryGetAllMileageSubstitutionHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllMileageSubstitutionHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllMileageSubstitutionHistorical"));
std::string QueryGetAllMileageSubstitutionHistorical::_baseSQL;
bool QueryGetAllMileageSubstitutionHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllMileageSubstitutionHistorical>
g_GetAllMileageSubstitutionHistorical;

const char*
QueryGetAllMileageSubstitutionHistorical::getQueryName() const
{
  return "GETALLMILEAGESUBSTITUTIONHISTORICAL";
}

void
QueryGetAllMileageSubstitutionHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllMileageSubstitutionHistoricalSQLStatement<QueryGetAllMileageSubstitutionHistorical>
    sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLMILEAGESUBSTITUTIONHISTORICAL");

    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllMileageSubstitutionHistorical::findAllMileageSubstitution(
    std::vector<tse::MileageSubstitution*>& lstMS)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstMS.push_back(QueryGetAllMileageSubstitutionHistoricalSQLStatement<
        QueryGetAllMileageSubstitutionHistorical>::mapRowToMileageSubstitution(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLMILEAGESUBSTITUTIONHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllMileageSubstitution()
}
