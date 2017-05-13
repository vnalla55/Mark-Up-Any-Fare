//----------------------------------------------------------------------------
//  File:           QueryGetFareDispInclFareType.cpp
//  Description:    QueryGetFareDispInclFareType
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

#include "DBAccess/Queries/QueryGetFareDispInclFareType.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareDispInclFareTypeSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetFareDispInclFareType::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetFareDispInclFareType"));
std::string QueryGetFareDispInclFareType::_baseSQL;
bool QueryGetFareDispInclFareType::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFareDispInclFareType> g_GetFareDispInclFareType;

const char*
QueryGetFareDispInclFareType::getQueryName() const
{
  return "GETFAREDISPINCLFARETYPE";
}

void
QueryGetFareDispInclFareType::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareDispInclFareTypeSQLStatement<QueryGetFareDispInclFareType> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFAREDISPINCLFARETYPE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetFareDispInclFareType::findFareDispInclFareType(
    std::vector<tse::FareDispInclFareType*>& fareDispInclFareTypes,
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
    fareDispInclFareTypes.push_back(QueryGetFareDispInclFareTypeSQLStatement<
        QueryGetFareDispInclFareType>::mapRowToFareDispInclFareType(row));
  }
  LOG4CXX_INFO(_logger,
               "GETFAREDISPINCLFARETYPE: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                     << " (" << stopCPU() << ")");
  res.freeResult();
} // findFareDispInclFareType()

///////////////////////////////////////////////////////////
//
//  QueryGetAllFareDispInclFareType
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllFareDispInclFareType::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllFareDispInclFareType"));
std::string QueryGetAllFareDispInclFareType::_baseSQL;
bool QueryGetAllFareDispInclFareType::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllFareDispInclFareType> g_GetAllFareDispInclFareType;

const char*
QueryGetAllFareDispInclFareType::getQueryName() const
{
  return "GETALLFAREDISPINCLFARETYPE";
}

void
QueryGetAllFareDispInclFareType::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllFareDispInclFareTypeSQLStatement<QueryGetAllFareDispInclFareType> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLFAREDISPINCLFARETYPE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllFareDispInclFareType::findAllFareDispInclFareType(
    std::vector<tse::FareDispInclFareType*>& fareDispInclFareTypes)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    fareDispInclFareTypes.push_back(QueryGetAllFareDispInclFareTypeSQLStatement<
        QueryGetAllFareDispInclFareType>::mapRowToFareDispInclFareType(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLFAREDISPINCLFARETYPE: NumRows = " << res.numRows() << " Time = "
                                                        << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllFareDispInclFareType()
}
