//----------------------------------------------------------------------------
//  File:           QueryGetDBEGlobalClass.cpp
//  Description:    QueryGetDBEGlobalClass
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

#include "DBAccess/Queries/QueryGetDBEGlobalClass.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetDBEGlobalClassSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetDBEGlobalClass::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetDBEGlobalClass"));
std::string QueryGetDBEGlobalClass::_baseSQL;
bool QueryGetDBEGlobalClass::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetDBEGlobalClass> g_GetDBEGlobalClass;

const char*
QueryGetDBEGlobalClass::getQueryName() const
{
  return "GETDBEGLOBALCLASS";
}

void
QueryGetDBEGlobalClass::initialize()
{
  if (!_isInitialized)
  {
    QueryGetDBEGlobalClassSQLStatement<QueryGetDBEGlobalClass> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETDBEGLOBALCLASS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetDBEGlobalClass::findDBEGlobalClass(std::vector<tse::DBEGlobalClass*>& lstDBE,
                                           const DBEClass& dbeClass)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, dbeClass);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::DBEGlobalClass* dbePrev = nullptr;
  tse::DBEGlobalClass* dbe = nullptr;
  while ((row = res.nextRow()) != nullptr)
  {
    dbe = QueryGetDBEGlobalClassSQLStatement<QueryGetDBEGlobalClass>::mapRowToDBEGlobalClass(
        row, dbePrev);
    if (dbe != dbePrev)
      lstDBE.push_back(dbe);

    dbePrev = dbe;
  }
  LOG4CXX_INFO(_logger,
               "GETDBEGLOBALCLASS: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                               << stopCPU() << ")");
  res.freeResult();
} // findDBEGlobalClass()

///////////////////////////////////////////////////////////
//
//  QueryGetAllDBEGlobalClass
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllDBEGlobalClass::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllDBEGlobalClass"));
std::string QueryGetAllDBEGlobalClass::_baseSQL;
bool QueryGetAllDBEGlobalClass::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllDBEGlobalClass> g_GetAllDBEGlobalClass;

const char*
QueryGetAllDBEGlobalClass::getQueryName() const
{
  return "GETALLDBEGLOBALCLASS";
}

void
QueryGetAllDBEGlobalClass::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllDBEGlobalClassSQLStatement<QueryGetAllDBEGlobalClass> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLDBEGLOBALCLASS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllDBEGlobalClass::findAllDBEGlobalClass(std::vector<tse::DBEGlobalClass*>& lstDBE)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::DBEGlobalClass* dbePrev = nullptr;
  tse::DBEGlobalClass* dbe = nullptr;
  while ((row = res.nextRow()) != nullptr)
  {
    dbe = QueryGetAllDBEGlobalClassSQLStatement<QueryGetAllDBEGlobalClass>::mapRowToDBEGlobalClass(
        row, dbePrev);
    if (dbe != dbePrev)
      lstDBE.push_back(dbe);

    dbePrev = dbe;
  }
  LOG4CXX_INFO(_logger,
               "GETALLDBEGLOBALCLASS: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                  << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllDBEGlobalClass()
}
