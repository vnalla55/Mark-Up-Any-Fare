//----------------------------------------------------------------------------
//  File:           QueryGetFDSGlobalDir.cpp
//  Description:    QueryGetFDSGlobalDir
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
#include "DBAccess/Queries/QueryGetFDSGlobalDir.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFDSGlobalDirSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetFDSGlobalDir::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetFDSGlobalDir"));
std::string QueryGetFDSGlobalDir::_baseSQL;
bool QueryGetFDSGlobalDir::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFDSGlobalDir> g_GetFDSGlobalDir;

const char*
QueryGetFDSGlobalDir::getQueryName() const
{
  return "GETFDSGLOBALDIR";
}

void
QueryGetFDSGlobalDir::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFDSGlobalDirSQLStatement<QueryGetFDSGlobalDir> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFDSGLOBALDIR");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetFDSGlobalDir::findFDSGlobalDir(std::vector<tse::FDSGlobalDir*>& infos,
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
    infos.push_back(
        QueryGetFDSGlobalDirSQLStatement<QueryGetFDSGlobalDir>::mapRowToFDSGlobalDir(row));
  }
  LOG4CXX_INFO(_logger,
               "GETFDSGLOBALDIR: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                             << stopCPU() << ")");
  res.freeResult();
} // findFDSGlobalDir()

///////////////////////////////////////////////////////////
//
//  QueryGetAllFDSGlobalDir()
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllFDSGlobalDir::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllFDSGlobalDir"));
std::string QueryGetAllFDSGlobalDir::_baseSQL;
bool QueryGetAllFDSGlobalDir::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllFDSGlobalDir> g_GetAllFDSGlobalDir;

const char*
QueryGetAllFDSGlobalDir::getQueryName() const
{
  return "GETALLFDSGLOBALDIR";
}

void
QueryGetAllFDSGlobalDir::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllFDSGlobalDirSQLStatement<QueryGetAllFDSGlobalDir> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLFDSGLOBALDIR");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllFDSGlobalDir::findAllFDSGlobalDir(std::vector<tse::FDSGlobalDir*>& infos)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    infos.push_back(
        QueryGetAllFDSGlobalDirSQLStatement<QueryGetAllFDSGlobalDir>::mapRowToFDSGlobalDir(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLFDSGLOBALDIR: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllFDSGlobalDir()
}
