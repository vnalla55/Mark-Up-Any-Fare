//----------------------------------------------------------------------------
//  File:           QueryGetNoPNROptions.cpp
//  Description:    QueryGetNoPNROptions
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

#include "DBAccess/Queries/QueryGetNoPNROptions.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetNoPNROptionsSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetNoPNROptions::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetNoPNROptions"));
std::string QueryGetNoPNROptions::_baseSQL;
bool QueryGetNoPNROptions::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetNoPNROptions> g_GetNoPNROptions;

const char*
QueryGetNoPNROptions::getQueryName() const
{
  return "GETNOPNROPTIONS";
}

void
QueryGetNoPNROptions::initialize()
{
  if (!_isInitialized)
  {
    QueryGetNoPNROptionsSQLStatement<QueryGetNoPNROptions> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETNOPNROPTIONS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetNoPNROptions::findNoPNROptions(std::vector<tse::NoPNROptions*>& npos,
                                       const Indicator userApplType,
                                       const UserApplCode& userAppl)
{
  // return;

  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, userApplType);
  substParm(2, userAppl);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::NoPNROptions* npo = nullptr;
  tse::NoPNROptions* npoPrev = nullptr;
  while ((row = res.nextRow()))
  {
    npo =
        QueryGetNoPNROptionsSQLStatement<QueryGetNoPNROptions>::mapRowToNoPNROptions(row, npoPrev);
    if (npo != npoPrev)
    {
      npos.push_back(npo);
    }
    npoPrev = npo;
  }
  LOG4CXX_INFO(_logger,
               "GETNOPNROPTIONS: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                             << stopCPU() << ")");
  res.freeResult();
} // findNoPNROptions()

///////////////////////////////////////////////////////////
//
//  QueryGetAllNoPNROptions
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllNoPNROptions::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllNoPNROptions"));
std::string QueryGetAllNoPNROptions::_baseSQL;
bool QueryGetAllNoPNROptions::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllNoPNROptions> g_GetAllNoPNROptions;

const char*
QueryGetAllNoPNROptions::getQueryName() const
{
  return "GETALLNOPNROPTIONS";
}

void
QueryGetAllNoPNROptions::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllNoPNROptionsSQLStatement<QueryGetAllNoPNROptions> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLNOPNROPTIONS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllNoPNROptions::findAllNoPNROptions(std::vector<tse::NoPNROptions*>& npos)
{
  // return;

  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::NoPNROptions* npo = nullptr;
  tse::NoPNROptions* npoPrev = nullptr;
  while ((row = res.nextRow()))
  {
    npo = QueryGetAllNoPNROptionsSQLStatement<QueryGetAllNoPNROptions>::mapRowToNoPNROptions(
        row, npoPrev);
    if (npo != npoPrev)
    {
      npos.push_back(npo);
    }
    npoPrev = npo;
  }
  LOG4CXX_INFO(_logger,
               "GETALLNOPNROPTIONS: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllNoPNROptions()
}
