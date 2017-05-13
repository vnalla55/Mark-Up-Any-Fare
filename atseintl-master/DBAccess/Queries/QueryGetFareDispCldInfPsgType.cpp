//----------------------------------------------------------------------------
//  File:           QueryGetFareDispCldInfPsgType.cpp
//  Description:    QueryGetFareDispCldInfPsgType
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

#include "DBAccess/Queries/QueryGetFareDispCldInfPsgType.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareDispCldInfPsgTypeSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetFareDispCldInfPsgType::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetFareDispCldInfPsgType"));
std::string QueryGetFareDispCldInfPsgType::_baseSQL;
bool QueryGetFareDispCldInfPsgType::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFareDispCldInfPsgType> g_GetFareDispCldInfPsgType;

const char*
QueryGetFareDispCldInfPsgType::getQueryName() const
{
  return "GETFAREDISPCLDINFPSGTYPE";
}

void
QueryGetFareDispCldInfPsgType::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareDispCldInfPsgTypeSQLStatement<QueryGetFareDispCldInfPsgType> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFAREDISPCLDINFPSGTYPE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetFareDispCldInfPsgType::findFareDispCldInfPsgType(
    std::vector<tse::FareDispCldInfPsgType*>& infos,
    const Indicator& userApplType,
    const UserApplCode& userAppl,
    const Indicator& pseudoCityType,
    const PseudoCityCode& pseudoCity,
    const InclusionCode& inclusionCode)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, userApplType);
  substParm(2, userAppl);
  substParm(3, pseudoCityType);
  substParm(4, pseudoCity);
  substParm(5, inclusionCode);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    infos.push_back(QueryGetFareDispCldInfPsgTypeSQLStatement<
        QueryGetFareDispCldInfPsgType>::mapRowToFareDispCldInfPsg(row));
  }
  LOG4CXX_INFO(_logger,
               "GETFAREDISPCLDINFPSGTYPE: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ")");
  res.freeResult();
} // findFareDispCldInfPsgType()

///////////////////////////////////////////////////////////
//
//  QueryGetAllFareDispCldInfPsgType()
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllFareDispCldInfPsgType::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllFareDispCldInfPsgType"));
std::string QueryGetAllFareDispCldInfPsgType::_baseSQL;
bool QueryGetAllFareDispCldInfPsgType::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllFareDispCldInfPsgType> g_GetAllFareDispCldInfPsgType;

const char*
QueryGetAllFareDispCldInfPsgType::getQueryName() const
{
  return "GETALLFAREDISPCLDINFPSGTYPE";
}

void
QueryGetAllFareDispCldInfPsgType::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllFareDispCldInfPsgTypeSQLStatement<QueryGetAllFareDispCldInfPsgType> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLFAREDISPCLDINFPSGTYPE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllFareDispCldInfPsgType::findAllFareDispCldInfPsgType(
    std::vector<tse::FareDispCldInfPsgType*>& infos)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    infos.push_back(QueryGetAllFareDispCldInfPsgTypeSQLStatement<
        QueryGetAllFareDispCldInfPsgType>::mapRowToFareDispCldInfPsg(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLFAREDISPCLDINFPSGTYPE: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
}
}
