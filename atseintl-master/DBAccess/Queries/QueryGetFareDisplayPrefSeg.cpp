//----------------------------------------------------------------------------
//  File:           QueryGetFareDisplayPrefSeg.cpp
//  Description:    QueryGetFareDisplayPrefSeg
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

#include "DBAccess/Queries/QueryGetFareDisplayPrefSeg.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareDisplayPrefSegSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetFareDisplayPrefSeg::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetFareDisplayPrefSeg"));
std::string QueryGetFareDisplayPrefSeg::_baseSQL;
bool QueryGetFareDisplayPrefSeg::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFareDisplayPrefSeg> g_GetFareDisplayPrefSeg;

const char*
QueryGetFareDisplayPrefSeg::getQueryName() const
{
  return "GETFAREDISPLAYPREFSEG";
}

void
QueryGetFareDisplayPrefSeg::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareDisplayPrefSegSQLStatement<QueryGetFareDisplayPrefSeg> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFAREDISPLAYPREFSEG");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetFareDisplayPrefSeg::findFareDisplayPrefSeg(
    std::vector<tse::FareDisplayPrefSeg*>& fareDisplayPrefSegs,
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
    fareDisplayPrefSegs.push_back(QueryGetFareDisplayPrefSegSQLStatement<
        QueryGetFareDisplayPrefSeg>::mapRowToFareDisplayPrefSeg(row));
  }
  LOG4CXX_INFO(_logger,
               "GETFAREDISPLAYPREFSEG: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                   << " (" << stopCPU() << ")");
  res.freeResult();
} // findFareDisplayPrefSeg()

///////////////////////////////////////////////////////////
//
//  QueryGetAllFareDisplayPrefSeg()
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllFareDisplayPrefSeg::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllFareDisplayPrefSeg"));
std::string QueryGetAllFareDisplayPrefSeg::_baseSQL;
bool QueryGetAllFareDisplayPrefSeg::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllFareDisplayPrefSeg> g_GetAllFareDisplayPrefSeg;

const char*
QueryGetAllFareDisplayPrefSeg::getQueryName() const
{
  return "GETALLFAREDISPLAYPREFSEG";
}

void
QueryGetAllFareDisplayPrefSeg::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllFareDisplayPrefSegSQLStatement<QueryGetAllFareDisplayPrefSeg> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLFAREDISPLAYPREFSEG");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllFareDisplayPrefSeg::findAllFareDisplayPrefSeg(
    std::vector<tse::FareDisplayPrefSeg*>& fareDisplayPrefSegs)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    fareDisplayPrefSegs.push_back(QueryGetAllFareDisplayPrefSegSQLStatement<
        QueryGetAllFareDisplayPrefSeg>::mapRowToFareDisplayPrefSeg(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLFAREDISPLAYPREFSEG: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllFareDisplayPrefSeg()
}
