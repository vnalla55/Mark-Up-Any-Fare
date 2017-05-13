//----------------------------------------------------------------------------
//  File:           QueryGetFareDispInclRuleTrf.cpp
//  Description:    QueryGetFareDispInclRuleTrf
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

#include "DBAccess/Queries/QueryGetFareDispInclRuleTrf.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareDispInclRuleTrfSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetFareDispInclRuleTrf::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetFareDispInclRuleTrf"));
std::string QueryGetFareDispInclRuleTrf::_baseSQL;
bool QueryGetFareDispInclRuleTrf::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFareDispInclRuleTrf> g_GetFareDispInclRuleTrf;

const char*
QueryGetFareDispInclRuleTrf::getQueryName() const
{
  return "GETFAREDISPINCLRULETRF";
}

void
QueryGetFareDispInclRuleTrf::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareDispInclRuleTrfSQLStatement<QueryGetFareDispInclRuleTrf> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFAREDISPINCLRULETRF");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetFareDispInclRuleTrf::findFareDispInclRuleTrf(
    std::vector<tse::FareDispInclRuleTrf*>& fareDispInclRuleTrfs,
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
    fareDispInclRuleTrfs.push_back(QueryGetFareDispInclRuleTrfSQLStatement<
        QueryGetFareDispInclRuleTrf>::mapRowToFareDispInclRuleTrf(row));
  }
  LOG4CXX_INFO(_logger,
               "GETFAREDISPINCLRULETRF: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                    << " (" << stopCPU() << ")");
  res.freeResult();
} // findFareDispInclRuleTrf()

///////////////////////////////////////////////////////////
//
//  QueryGetAllFareDispInclRuleTrf
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllFareDispInclRuleTrf::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllFareDispInclRuleTrf"));
std::string QueryGetAllFareDispInclRuleTrf::_baseSQL;
bool QueryGetAllFareDispInclRuleTrf::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllFareDispInclRuleTrf> g_GetAllFareDispInclRuleTrf;

const char*
QueryGetAllFareDispInclRuleTrf::getQueryName() const
{
  return "GETALLFAREDISPINCLRULETRF";
}

void
QueryGetAllFareDispInclRuleTrf::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllFareDispInclRuleTrfSQLStatement<QueryGetAllFareDispInclRuleTrf> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLFAREDISPINCLRULETRF");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllFareDispInclRuleTrf::findAllFareDispInclRuleTrf(
    std::vector<tse::FareDispInclRuleTrf*>& fareDispInclRuleTrfs)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    fareDispInclRuleTrfs.push_back(QueryGetAllFareDispInclRuleTrfSQLStatement<
        QueryGetAllFareDispInclRuleTrf>::mapRowToFareDispInclRuleTrf(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLFAREDISPINCLRULETRF: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                       << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllFareDispInclRuleTrf()
}
