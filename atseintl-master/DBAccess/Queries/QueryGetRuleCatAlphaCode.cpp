//----------------------------------------------------------------------------
//  File:           QueryGetRuleCatAlphaCode.cpp
//  Description:    QueryGetRuleCatAlphaCode
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

#include "DBAccess/Queries/QueryGetRuleCatAlphaCode.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetRuleCatAlphaCodeSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetRuleCatAlphaCode::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetRuleCatAlphaCode"));
std::string QueryGetRuleCatAlphaCode::_baseSQL;
bool QueryGetRuleCatAlphaCode::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetRuleCatAlphaCode> g_GetRuleCatAlphaCode;

const char*
QueryGetRuleCatAlphaCode::getQueryName() const
{
  return "GETRULECATALPHACODE";
}

void
QueryGetRuleCatAlphaCode::initialize()
{
  if (!_isInitialized)
  {
    QueryGetRuleCatAlphaCodeSQLStatement<QueryGetRuleCatAlphaCode> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETRULECATALPHACODE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetRuleCatAlphaCode::findRuleCatAlphaCode(std::vector<tse::RuleCatAlphaCode*>& infos,
                                               const AlphaCode& alphaCode)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, alphaCode);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    infos.push_back(
        QueryGetRuleCatAlphaCodeSQLStatement<QueryGetRuleCatAlphaCode>::mapRowToRuleCatAlphaCode(
            row));
  }
  LOG4CXX_INFO(_logger,
               "GETRULECATALPHACODE: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                 << " (" << stopCPU() << ")");
  res.freeResult();
} // findRuleCatAlphaCode()

///////////////////////////////////////////////////////////
//
//  QueryGetAllRuleCatAlphaCode()
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllRuleCatAlphaCode::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllRuleCatAlphaCode"));
std::string QueryGetAllRuleCatAlphaCode::_baseSQL;
bool QueryGetAllRuleCatAlphaCode::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllRuleCatAlphaCode> g_GetAllRuleCatAlphaCode;

const char*
QueryGetAllRuleCatAlphaCode::getQueryName() const
{
  return "GETALLRULECATALPHACODE";
}

void
QueryGetAllRuleCatAlphaCode::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllRuleCatAlphaCodeSQLStatement<QueryGetAllRuleCatAlphaCode> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLRULECATALPHACODE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllRuleCatAlphaCode::findAllRuleCatAlphaCode(std::vector<tse::RuleCatAlphaCode*>& infos)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    infos.push_back(QueryGetAllRuleCatAlphaCodeSQLStatement<
        QueryGetAllRuleCatAlphaCode>::mapRowToRuleCatAlphaCode(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLRULECATALPHACODE: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                    << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllRuleCatAlphaCode()
}
