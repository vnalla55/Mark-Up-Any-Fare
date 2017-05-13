//----------------------------------------------------------------------------
//  File:           QueryGetFareDispRec1PsgType.cpp
//  Description:    QueryGetFareDispRec1PsgType
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

#include "DBAccess/Queries/QueryGetFareDispRec1PsgType.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareDispRec1PsgTypeSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetFareDispRec1PsgType::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetFareDispRec1PsgType"));
std::string QueryGetFareDispRec1PsgType::_baseSQL;
bool QueryGetFareDispRec1PsgType::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFareDispRec1PsgType> g_GetFareDispRec1PsgType;

const char*
QueryGetFareDispRec1PsgType::getQueryName() const
{
  return "GETFAREDISPREC1PSGTYPE";
}

void
QueryGetFareDispRec1PsgType::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareDispRec1PsgTypeSQLStatement<QueryGetFareDispRec1PsgType> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFAREDISPREC1PSGTYPE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetFareDispRec1PsgType::findFareDispRec1PsgType(std::vector<tse::FareDispRec1PsgType*>& infos,
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
    infos.push_back(QueryGetFareDispRec1PsgTypeSQLStatement<
        QueryGetFareDispRec1PsgType>::mapRowToFareDispRec1PsgType(row));
  }
  LOG4CXX_INFO(_logger,
               "GETFAREDISPREC1PSGTYPE: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                    << " (" << stopCPU() << ")");
  res.freeResult();
} // findFareDispRec1PsgType()

///////////////////////////////////////////////////////////
//
//  QueryGetAllFareDispRec1PsgType()
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllFareDispRec1PsgType::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllFareDispRec1PsgType"));
std::string QueryGetAllFareDispRec1PsgType::_baseSQL;
bool QueryGetAllFareDispRec1PsgType::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllFareDispRec1PsgType> g_GetAllFareDispRec1PsgType;

const char*
QueryGetAllFareDispRec1PsgType::getQueryName() const
{
  return "GETALLFAREDISPREC1PSGTYPE";
}

void
QueryGetAllFareDispRec1PsgType::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllFareDispRec1PsgTypeSQLStatement<QueryGetAllFareDispRec1PsgType> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLFAREDISPREC1PSGTYPE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllFareDispRec1PsgType::findAllFareDispRec1PsgType(
    std::vector<tse::FareDispRec1PsgType*>& infos)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    infos.push_back(QueryGetAllFareDispRec1PsgTypeSQLStatement<
        QueryGetAllFareDispRec1PsgType>::mapRowToFareDispRec1PsgType(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLFAREDISPREC1PSGTYPE: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                       << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllFareDispRec1PsgType()
}
