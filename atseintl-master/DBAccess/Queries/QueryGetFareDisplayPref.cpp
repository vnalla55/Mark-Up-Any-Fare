//----------------------------------------------------------------------------
//  File:           QueryGetFareDisplayPref.cpp
//  Description:    QueryGetFareDisplayPref
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

#include "DBAccess/Queries/QueryGetFareDisplayPref.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareDisplayPrefSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetFareDisplayPref::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetFareDisplayPref"));
std::string QueryGetFareDisplayPref::_baseSQL;
bool QueryGetFareDisplayPref::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFareDisplayPref> g_GetFareDisplayPref;

const char*
QueryGetFareDisplayPref::getQueryName() const
{
  return "GETFAREDISPLAYPREF";
}

void
QueryGetFareDisplayPref::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareDisplayPrefSQLStatement<QueryGetFareDisplayPref> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFAREDISPLAYPREF");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetFareDisplayPref::findFareDisplayPref(std::vector<tse::FareDisplayPref*>& fareDisplayPrefs,
                                             const Indicator& userApplType,
                                             const UserApplCode& userAppl,
                                             const Indicator& pseudoCityType,
                                             const PseudoCityCode& pseudoCity,
                                             const TJRGroup& tjrGroup)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, userApplType);
  substParm(2, userAppl);
  substParm(3, pseudoCityType);
  substParm(4, pseudoCity);
  substParm(5, tjrGroup);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    fareDisplayPrefs.push_back(
        QueryGetFareDisplayPrefSQLStatement<QueryGetFareDisplayPref>::mapRowToFareDisplayPref(row));
  }
  LOG4CXX_INFO(_logger,
               "GETFAREDISPLAYPREF: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                << " (" << stopCPU() << ")");
  res.freeResult();
} // findFareDisplayPref()

///////////////////////////////////////////////////////////
//
//  QueryGetAllFareDisplayPref
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllFareDisplayPref::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllFareDisplayPref"));
std::string QueryGetAllFareDisplayPref::_baseSQL;
bool QueryGetAllFareDisplayPref::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllFareDisplayPref> g_GetAllFareDisplayPref;

const char*
QueryGetAllFareDisplayPref::getQueryName() const
{
  return "GETALLFAREDISPLAYPREF";
}

void
QueryGetAllFareDisplayPref::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllFareDisplayPrefSQLStatement<QueryGetAllFareDisplayPref> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLFAREDISPLAYPREF");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllFareDisplayPref::findAllFareDisplayPref(
    std::vector<tse::FareDisplayPref*>& fareDisplayPrefs)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    fareDisplayPrefs.push_back(
        QueryGetAllFareDisplayPrefSQLStatement<QueryGetAllFareDisplayPref>::mapRowToFareDisplayPref(
            row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLFAREDISPLAYPREF: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                   << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllFareDisplayPref()
}
