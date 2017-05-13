//----------------------------------------------------------------------------
//  File:           QueryGetFareDispInclDsplType.cpp
//  Description:    QueryGetFareDispInclDsplType
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

#include "DBAccess/Queries/QueryGetFareDispInclDsplType.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareDispInclDsplTypeSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetFareDispInclDsplType::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetFareDispInclDsplType"));
std::string QueryGetFareDispInclDsplType::_baseSQL;
bool QueryGetFareDispInclDsplType::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFareDispInclDsplType> g_GetFareDispInclDsplType;

const char*
QueryGetFareDispInclDsplType::getQueryName() const
{
  return "GETFAREDISPINCLDSPLTYPE";
}

void
QueryGetFareDispInclDsplType::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareDispInclDsplTypeSQLStatement<QueryGetFareDispInclDsplType> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFAREDISPINCLDSPLTYPE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetFareDispInclDsplType::findFareDispInclDsplType(
    std::vector<tse::FareDispInclDsplType*>& fareDispInclDsplTypes,
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
    fareDispInclDsplTypes.push_back(QueryGetFareDispInclDsplTypeSQLStatement<
        QueryGetFareDispInclDsplType>::mapRowToFareDispInclDsplType(row));
  }
  LOG4CXX_INFO(_logger,
               "GETFAREDISPINCLDSPLTYPE: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                     << " (" << stopCPU() << ")");
  res.freeResult();
} // findFareDispInclDsplType()

///////////////////////////////////////////////////////////
//
//  QueryGetAllFareDispInclDsplType
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllFareDispInclDsplType::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllFareDispInclDsplType"));
std::string QueryGetAllFareDispInclDsplType::_baseSQL;
bool QueryGetAllFareDispInclDsplType::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllFareDispInclDsplType> g_GetAllFareDispInclDsplType;

const char*
QueryGetAllFareDispInclDsplType::getQueryName() const
{
  return "GETALLFAREDISPINCLDSPLTYPE";
}

void
QueryGetAllFareDispInclDsplType::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllFareDispInclDsplTypeSQLStatement<QueryGetAllFareDispInclDsplType> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLFAREDISPINCLDSPLTYPE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllFareDispInclDsplType::findAllFareDispInclDsplType(
    std::vector<tse::FareDispInclDsplType*>& fareDispInclDsplTypes)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    fareDispInclDsplTypes.push_back(QueryGetAllFareDispInclDsplTypeSQLStatement<
        QueryGetAllFareDispInclDsplType>::mapRowToFareDispInclDsplType(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLFAREDISPINCLDSPLTYPE: NumRows = " << res.numRows() << " Time = "
                                                        << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllFareDispInclDsplType()
}
