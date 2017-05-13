//----------------------------------------------------------------------------
//  File:           QueryGetNoPNRFareTypeGroup.cpp
//  Description:    QueryGetNoPNRFareTypeGroup
//  Created:        1/11/2008
// Authors:         Karolina Golebiewska
//
//  Updates:
//
// � 2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetNoPNRFareTypeGroup.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetNoPNRFareTypeGroupSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetNoPNRFareTypeGroup::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetNoPNRFareTypeGroup"));
std::string QueryGetNoPNRFareTypeGroup::_baseSQL;
bool QueryGetNoPNRFareTypeGroup::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetNoPNRFareTypeGroup> g_GetNoPNRFareTypeGroup;

const char*
QueryGetNoPNRFareTypeGroup::getQueryName() const
{
  return "GETNOPNRFARETYPEGROUP";
}

void
QueryGetNoPNRFareTypeGroup::initialize()
{
  if (!_isInitialized)
  {
    QueryGetNoPNRFareTypeGroupSQLStatement<QueryGetNoPNRFareTypeGroup> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETNOPNRFARETYPEGROUP");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetNoPNRFareTypeGroup::findNoPNRFareTypeGroup(std::vector<tse::NoPNRFareTypeGroup*>& npftgs,
                                                   const int fareTypeGroup)
{
  // return;

  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, fareTypeGroup);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::NoPNRFareTypeGroup* npftg = nullptr;
  tse::NoPNRFareTypeGroup* npftgPrev = nullptr;
  while ((row = res.nextRow()))
  {
    npftg = QueryGetNoPNRFareTypeGroupSQLStatement<
        QueryGetNoPNRFareTypeGroup>::mapRowToNoPNRFareTypeGroup(row, npftgPrev);
    if (npftg != npftgPrev)
    {
      npftgs.push_back(npftg);
    }
    npftgPrev = npftg;
  }
  LOG4CXX_INFO(_logger,
               "GETNOPNRFARETYPEGROUP: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                   << " (" << stopCPU() << ")");
  res.freeResult();
} // findNoPNRFareTypeGroup()

///////////////////////////////////////////////////////////
//
//  QueryGetAllNoPNRFareTypeGroup
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllNoPNRFareTypeGroup::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllNoPNRFareTypeGroup"));
std::string QueryGetAllNoPNRFareTypeGroup::_baseSQL;
bool QueryGetAllNoPNRFareTypeGroup::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllNoPNRFareTypeGroup> g_GetAllNoPNRFareTypeGroup;

const char*
QueryGetAllNoPNRFareTypeGroup::getQueryName() const
{
  return "GETALLNOPNRFARETYPEGROUP";
}

void
QueryGetAllNoPNRFareTypeGroup::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllNoPNRFareTypeGroupSQLStatement<QueryGetAllNoPNRFareTypeGroup> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLNOPNRFARETYPEGROUP");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllNoPNRFareTypeGroup::findAllNoPNRFareTypeGroup(
    std::vector<tse::NoPNRFareTypeGroup*>& npftgs)
{
  // return;

  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::NoPNRFareTypeGroup* npftg = nullptr;
  tse::NoPNRFareTypeGroup* npftgPrev = nullptr;
  while ((row = res.nextRow()))
  {
    npftg = QueryGetAllNoPNRFareTypeGroupSQLStatement<
        QueryGetAllNoPNRFareTypeGroup>::mapRowToNoPNRFareTypeGroup(row, npftgPrev);
    if (npftg != npftgPrev)
    {
      npftgs.push_back(npftg);
    }
    npftgPrev = npftg;
  }
  LOG4CXX_INFO(_logger,
               "GETALLNOPNRFARETYPEGROUP: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllNoPNRFareTypeGroup()
}
