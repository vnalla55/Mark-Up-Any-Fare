//----------------------------------------------------------------------------
//  File:           QueryGetFareDisplayWeb.cpp
//  Description:    QueryGetFareDisplayWeb
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

#include "DBAccess/Queries/QueryGetFareDisplayWeb.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareDisplayWebSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetFareDisplayWeb::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetFareDisplayWeb"));
std::string QueryGetFareDisplayWeb::_baseSQL;
bool QueryGetFareDisplayWeb::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFareDisplayWeb> g_GetFareDisplayWeb;

const char*
QueryGetFareDisplayWeb::getQueryName() const
{
  return "GETFAREDISPLAYWEB";
}

void
QueryGetFareDisplayWeb::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareDisplayWebSQLStatement<QueryGetFareDisplayWeb> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFAREDISPLAYWEB");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetFareDisplayWeb::findFareDisplayWeb(std::vector<tse::FareDisplayWeb*>& fareDisplayWebs,
                                           const CarrierCode& carrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, carrier);

  LOG4CXX_INFO(_logger, "QUERY: " << this->getSQL());

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    fareDisplayWebs.push_back(
        QueryGetFareDisplayWebSQLStatement<QueryGetFareDisplayWeb>::mapRowToFareDisplayWeb(row));
  }
  LOG4CXX_INFO(_logger,
               "GETFAREDISPLAYWEB: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                               << stopCPU() << ")");
  res.freeResult();
} // findFareDisplayWeb()

///////////////////////////////////////////////////////////
//
//  QueryGetAllFareDisplayWeb()
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllFareDisplayWeb::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllFareDisplayWeb"));
std::string QueryGetAllFareDisplayWeb::_baseSQL;
bool QueryGetAllFareDisplayWeb::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllFareDisplayWeb> g_GetAllFareDisplayWeb;

const char*
QueryGetAllFareDisplayWeb::getQueryName() const
{
  return "GETALLFAREDISPLAYWEB";
}

void
QueryGetAllFareDisplayWeb::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllFareDisplayWebSQLStatement<QueryGetAllFareDisplayWeb> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLFAREDISPLAYWEB");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllFareDisplayWeb::findAllFareDisplayWeb(std::vector<tse::FareDisplayWeb*>& fareDisplayWebs)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    fareDisplayWebs.push_back(
        QueryGetAllFareDisplayWebSQLStatement<QueryGetAllFareDisplayWeb>::mapRowToFareDisplayWeb(
            row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLFAREDISPLAYWEB: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                  << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllFareDisplayWeb()
}
