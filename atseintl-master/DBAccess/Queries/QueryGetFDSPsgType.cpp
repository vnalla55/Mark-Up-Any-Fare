//----------------------------------------------------------------------------
//  File:           QueryGetFDSPsgType.cpp
//  Description:    QueryGetFDSPsgType
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
#include "DBAccess/Queries/QueryGetFDSPsgType.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFDSPsgTypeSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetFDSPsgType::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetFDSPsgType"));
std::string QueryGetFDSPsgType::_baseSQL;
bool QueryGetFDSPsgType::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFDSPsgType> g_GetFDSPsgType;

const char*
QueryGetFDSPsgType::getQueryName() const
{
  return "GETFDSPSGTYPE";
}

void
QueryGetFDSPsgType::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFDSPsgTypeSQLStatement<QueryGetFDSPsgType> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFDSPSGTYPE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetFDSPsgType::findFDSPsgType(std::vector<tse::FDSPsgType*>& infos,
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
    infos.push_back(QueryGetFDSPsgTypeSQLStatement<QueryGetFDSPsgType>::mapRowToFDSPsgType(row));
  }
  LOG4CXX_INFO(_logger,
               "GETFDSPSGTYPE: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                           << stopCPU() << ")");
  res.freeResult();
} // findFDSPsgType()

///////////////////////////////////////////////////////////
//
//  QueryGetAllFDSPsgType()
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllFDSPsgType::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllFDSPsgType"));
std::string QueryGetAllFDSPsgType::_baseSQL;
bool QueryGetAllFDSPsgType::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllFDSPsgType> g_GetAllFDSPsgType;

const char*
QueryGetAllFDSPsgType::getQueryName() const
{
  return "GETALLFDSPSGTYPE";
}

void
QueryGetAllFDSPsgType::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllFDSPsgTypeSQLStatement<QueryGetAllFDSPsgType> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLFDSPSGTYPE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllFDSPsgType::findAllFDSPsgType(std::vector<tse::FDSPsgType*>& infos)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    infos.push_back(
        QueryGetAllFDSPsgTypeSQLStatement<QueryGetAllFDSPsgType>::mapRowToFDSPsgType(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLFDSPSGTYPE: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                              << stopCPU() << ")");
  res.freeResult();
} // findAllFDSPsgType()
}
