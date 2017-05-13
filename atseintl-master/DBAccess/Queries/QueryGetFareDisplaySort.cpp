//----------------------------------------------------------------------------
//  File:           QueryGetFareDisplaySort.cpp
//  Description:    QueryGetFareDisplaySort
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

#include "DBAccess/Queries/QueryGetFareDisplaySort.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareDisplaySortSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetFareDisplaySort::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetFareDisplaySort"));
std::string QueryGetFareDisplaySort::_baseSQL;
bool QueryGetFareDisplaySort::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFareDisplaySort> g_GetFareDisplaySort;

const char*
QueryGetFareDisplaySort::getQueryName() const
{
  return "GETFAREDISPLAYSORT";
}

void
QueryGetFareDisplaySort::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareDisplaySortSQLStatement<QueryGetFareDisplaySort> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFAREDISPLAYSORT");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetFareDisplaySort::findFareDisplaySort(std::vector<tse::FareDisplaySort*>& infos,
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
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    infos.push_back(
        QueryGetFareDisplaySortSQLStatement<QueryGetFareDisplaySort>::mapRowToFareDisplaySort(row));
  }
  LOG4CXX_INFO(_logger,
               "GETFAREDISPLAYSORT: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                << " (" << stopCPU() << ")");
  res.freeResult();
} // findFareDisplaySort()

///////////////////////////////////////////////////////////
//
//  QueryGetAllFareDisplaySort()
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllFareDisplaySort::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllFareDisplaySort"));
std::string QueryGetAllFareDisplaySort::_baseSQL;
bool QueryGetAllFareDisplaySort::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllFareDisplaySort> g_GetAllFareDisplaySort;

const char*
QueryGetAllFareDisplaySort::getQueryName() const
{
  return "GETALLFAREDISPLAYSORT";
}

void
QueryGetAllFareDisplaySort::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllFareDisplaySortSQLStatement<QueryGetAllFareDisplaySort> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLFAREDISPLAYSORT");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllFareDisplaySort::findAllFareDisplaySort(std::vector<tse::FareDisplaySort*>& infos)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    infos.push_back(
        QueryGetAllFareDisplaySortSQLStatement<QueryGetAllFareDisplaySort>::mapRowToFareDisplaySort(
            row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLFAREDISPLAYSORT: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                   << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllFareDisplaySort()
}
