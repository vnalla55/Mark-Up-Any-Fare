//----------------------------------------------------------------------------
//  File:           QueryGetFDSSorting.cpp
//  Description:    QueryGetFDSSorting
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
#include "DBAccess/Queries/QueryGetFDSSorting.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFDSSortingSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetFDSSorting::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetFDSSorting"));
std::string QueryGetFDSSorting::_baseSQL;
bool QueryGetFDSSorting::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFDSSorting> g_GetFDSSorting;

const char*
QueryGetFDSSorting::getQueryName() const
{
  return "GETFDSSORTING";
}

void
QueryGetFDSSorting::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFDSSortingSQLStatement<QueryGetFDSSorting> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFDSSORTING");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetFDSSorting::findFDSSorting(std::vector<tse::FDSSorting*>& infos,
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
    infos.push_back(QueryGetFDSSortingSQLStatement<QueryGetFDSSorting>::mapRowToFDSSorting(row));
  }
  LOG4CXX_INFO(_logger,
               "GETFDSSORTING: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                           << stopCPU() << ")");
  res.freeResult();
} // findFDSSorting()

///////////////////////////////////////////////////////////
//
//  QueryGetAllFDSSorting()
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllFDSSorting::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllFDSSorting"));
std::string QueryGetAllFDSSorting::_baseSQL;
bool QueryGetAllFDSSorting::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllFDSSorting> g_GetAllFDSSorting;

const char*
QueryGetAllFDSSorting::getQueryName() const
{
  return "GETALLFDSSORTING";
}

void
QueryGetAllFDSSorting::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllFDSSortingSQLStatement<QueryGetAllFDSSorting> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLFDSSORTING");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllFDSSorting::findAllFDSSorting(std::vector<tse::FDSSorting*>& infos)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    infos.push_back(
        QueryGetAllFDSSortingSQLStatement<QueryGetAllFDSSorting>::mapRowToFDSSorting(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLFDSSORTING: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                              << stopCPU() << ")");
  res.freeResult();
} // findAllFDSSorting()
}
