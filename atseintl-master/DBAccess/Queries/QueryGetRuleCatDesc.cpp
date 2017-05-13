//----------------------------------------------------------------------------
//  File:           QueryGetRuleCatDesc.cpp
//  Description:    QueryGetRuleCatDesc
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

#include "DBAccess/Queries/QueryGetRuleCatDesc.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetRuleCatDescSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetRuleCatDesc::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetRuleCatDesc"));
std::string QueryGetRuleCatDesc::_baseSQL;
bool QueryGetRuleCatDesc::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetRuleCatDesc> g_GetRuleCatDesc;

const char*
QueryGetRuleCatDesc::getQueryName() const
{
  return "GETRULECATDESC";
}

void
QueryGetRuleCatDesc::initialize()
{
  if (!_isInitialized)
  {
    QueryGetRuleCatDescSQLStatement<QueryGetRuleCatDesc> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETRULECATDESC");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetRuleCatDesc::findRuleCategoryDesc(tse::RuleCategoryDescInfo*& info,
                                          const CatNumber& catNumber)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, catNumber);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  if ((row = res.nextRow()))
  {
    info = QueryGetRuleCatDescSQLStatement<QueryGetRuleCatDesc>::mapRowToRuleCatDesc(row);
  }
  else
  {
    info = nullptr;
  }
  LOG4CXX_INFO(_logger,
               "GETRULECATDESC: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                            << stopCPU() << ")");
  res.freeResult();
} // findRuleCategoryDesc()

///////////////////////////////////////////////////////////
//
//  QueryGetAllRuleCatDescs()
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllRuleCatDescs::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllRuleCatDescs"));
std::string QueryGetAllRuleCatDescs::_baseSQL;
bool QueryGetAllRuleCatDescs::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllRuleCatDescs> g_GetAllRuleCatDescs;

const char*
QueryGetAllRuleCatDescs::getQueryName() const
{
  return "GETALLRULECATDESCS";
}

void
QueryGetAllRuleCatDescs::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllRuleCatDescSQLStatement<QueryGetAllRuleCatDescs> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLRULECATDESCS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllRuleCatDescs::findAllRuleCategoryDesc(std::vector<tse::RuleCategoryDescInfo*>& infos)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    infos.push_back(
        QueryGetAllRuleCatDescSQLStatement<QueryGetAllRuleCatDescs>::mapRowToRuleCatDesc(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLRULECATDESCS: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllRuleCategoryDesc()
}
