//----------------------------------------------------------------------------
//  File:           QueryGetFareDispRec8PsgType.cpp
//  Description:    QueryGetFareDispRec8PsgType
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

#include "DBAccess/Queries/QueryGetFareDispRec8PsgType.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareDispRec8PsgTypeSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetFareDispRec8PsgType::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetFareDispRec8PsgType"));
std::string QueryGetFareDispRec8PsgType::_baseSQL;
bool QueryGetFareDispRec8PsgType::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFareDispRec8PsgType> g_GetFareDispRec8PsgType;

const char*
QueryGetFareDispRec8PsgType::getQueryName() const
{
  return "GETFAREDISPREC8PSGTYPE";
}

void
QueryGetFareDispRec8PsgType::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareDispRec8PsgTypeSQLStatement<QueryGetFareDispRec8PsgType> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFAREDISPREC8PSGTYPE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetFareDispRec8PsgType::findFareDispRec8PsgType(std::vector<tse::FareDispRec8PsgType*>& infos,
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
    infos.push_back(QueryGetFareDispRec8PsgTypeSQLStatement<
        QueryGetFareDispRec8PsgType>::mapRowToFareDispRec8PsgType(row));
  }
  LOG4CXX_INFO(_logger,
               "GETFAREDISPREC8PSGTYPE: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                    << " (" << stopCPU() << ")");
  res.freeResult();
} // findFareDispRec8PsgType()

///////////////////////////////////////////////////////////
//
//  QueryGetAllFareDispRec8PsgType()
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllFareDispRec8PsgType::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllFareDispRec8PsgType"));
std::string QueryGetAllFareDispRec8PsgType::_baseSQL;
bool QueryGetAllFareDispRec8PsgType::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllFareDispRec8PsgType> g_GetAllFareDispRec8PsgType;

const char*
QueryGetAllFareDispRec8PsgType::getQueryName() const
{
  return "GETALLFAREDISPREC8PSGTYPE";
}

void
QueryGetAllFareDispRec8PsgType::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllFareDispRec8PsgTypeSQLStatement<QueryGetAllFareDispRec8PsgType> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLFAREDISPREC8PSGTYPE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllFareDispRec8PsgType::findAllFareDispRec8PsgType(
    std::vector<tse::FareDispRec8PsgType*>& infos)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    infos.push_back(QueryGetAllFareDispRec8PsgTypeSQLStatement<
        QueryGetAllFareDispRec8PsgType>::mapRowToFareDispRec8PsgType(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLFAREDISPREC8PSGTYPE: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                       << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllFareDispRec8PsgType()
}
