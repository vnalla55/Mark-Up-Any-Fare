//----------------------------------------------------------------------------
//  File:           QueryGetFareDisplayInclCd.cpp
//  Description:    QueryGetFareDisplayInclCd
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

#include "DBAccess/Queries/QueryGetFareDisplayInclCd.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareDisplayInclCdSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetFareDisplayInclCd::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetFareDisplayInclCd"));
std::string QueryGetFareDisplayInclCd::_baseSQL;
bool QueryGetFareDisplayInclCd::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFareDisplayInclCd> g_GetFareDisplayInclCd;

const char*
QueryGetFareDisplayInclCd::getQueryName() const
{
  return "GETFAREDISPLAYINCLCD";
}

void
QueryGetFareDisplayInclCd::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareDisplayInclCdSQLStatement<QueryGetFareDisplayInclCd> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFAREDISPLAYINCLCD");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetFareDisplayInclCd::findFareDisplayInclCd(
    std::vector<tse::FareDisplayInclCd*>& fareDisplayInclCds,
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
    fareDisplayInclCds.push_back(
        QueryGetFareDisplayInclCdSQLStatement<QueryGetFareDisplayInclCd>::mapRowToFareDisplayInclCd(
            row));
  }
  LOG4CXX_INFO(_logger,
               "GETFAREDISPLAYINCLCD: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                  << " (" << stopCPU() << ")");
  res.freeResult();
} // findFareDisplayInclCd()

///////////////////////////////////////////////////////////
//
//  QueryGetAllFareDisplayInclCd
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllFareDisplayInclCd::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllFareDisplayInclCd"));
std::string QueryGetAllFareDisplayInclCd::_baseSQL;
bool QueryGetAllFareDisplayInclCd::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllFareDisplayInclCd> g_GetAllFareDisplayInclCd;

const char*
QueryGetAllFareDisplayInclCd::getQueryName() const
{
  return "GETALLFAREDISPLAYINCLCD";
}

void
QueryGetAllFareDisplayInclCd::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllFareDisplayInclCdSQLStatement<QueryGetAllFareDisplayInclCd> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLFAREDISPLAYINCLCD");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllFareDisplayInclCd::findAllFareDisplayInclCd(
    std::vector<tse::FareDisplayInclCd*>& fareDisplayInclCds)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    fareDisplayInclCds.push_back(QueryGetAllFareDisplayInclCdSQLStatement<
        QueryGetAllFareDisplayInclCd>::mapRowToFareDisplayInclCd(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLFAREDISPLAYINCLCD: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                     << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllFareDisplayInclCd()
}
