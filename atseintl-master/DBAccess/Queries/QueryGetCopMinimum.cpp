//----------------------------------------------------------------------------
//  File:           QueryGetCopMinimum.cpp
//  Description:    QueryGetCopMinimum
//  Created:        8/24/2006
// Authors:         Mike Lillis
//
//  Updates:
//
// � 2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetCopMinimum.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCopMinimumSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetCopMinBase::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetCopMinBase"));
std::string QueryGetCopMinBase::_baseSQL;
bool QueryGetCopMinBase::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCopMinBase> g_GetCopMinBase;

log4cxx::LoggerPtr
QueryGetCopTktgCxrExcpts::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetCopMinBase.GetCopTktgCxrExcpts"));
std::string QueryGetCopTktgCxrExcpts::_baseSQL;
bool QueryGetCopTktgCxrExcpts::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCopTktgCxrExcpts> g_GetCopTktgCxrExcpts;

const char*
QueryGetCopMinBase::getQueryName() const
{
  return "GETCOPMINBASE";
};

void
QueryGetCopMinBase::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCopMinBaseSQLStatement<QueryGetCopMinBase> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCOPMINBASE");
    substTableDef(&_baseSQL);

    QueryGetCopTktgCxrExcpts::initialize();
    _isInitialized = true;
  }
} // initialize()

void
QueryGetCopMinBase::findCopMinimum(std::vector<tse::CopMinimum*>& lstCM, const NationCode& nation)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, nation);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::CopMinimum* cm = nullptr;
  tse::CopMinimum* cmPrev = nullptr;
  while ((row = res.nextRow()))
  {
    cm = QueryGetCopMinBaseSQLStatement<QueryGetCopMinBase>::mapRowToCopMinBase(row, cmPrev);
    if (cm != cmPrev)
      lstCM.push_back(cm);

    cmPrev = cm;
  }
  LOG4CXX_INFO(_logger,
               "GETCOPMINBASE: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                           << stopCPU() << ")");
  res.freeResult();

  // Get TktgCxrExcepts
  QueryGetCopTktgCxrExcpts SQLTktgCxrExcpts(_dbAdapt);
  std::vector<tse::CopMinimum*>::iterator i;
  for (i = lstCM.begin(); i != lstCM.end(); i++)
  {
    SQLTktgCxrExcpts.getTktgCxrExcpts(*i);
  }
} // QueryGetCopMinBase::findCopMinimum()

///////////////////////////////////////////////////////////
//  QueryGetCopMinBaseHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetCopMinBaseHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetCopMinBaseHistorical"));
std::string QueryGetCopMinBaseHistorical::_baseSQL;
bool QueryGetCopMinBaseHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCopMinBaseHistorical> g_GetCopMinBaseHistorical;

const char*
QueryGetCopMinBaseHistorical::getQueryName() const
{
  return "GETCOPMINBASEHISTORICAL";
};

void
QueryGetCopMinBaseHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCopMinBaseHistoricalSQLStatement<QueryGetCopMinBaseHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCOPMINBASEHISTORICAL");
    substTableDef(&_baseSQL);

    QueryGetCopTktgCxrExcpts::initialize();
    _isInitialized = true;
  }
} // initialize()

void
QueryGetCopMinBaseHistorical::findCopMinimum(std::vector<tse::CopMinimum*>& lstCM,
                                             const NationCode& nation)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(nation, 1);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::CopMinimum* cm = nullptr;
  tse::CopMinimum* cmPrev = nullptr;
  while ((row = res.nextRow()))
  {
    cm = QueryGetCopMinBaseHistoricalSQLStatement<QueryGetCopMinBaseHistorical>::mapRowToCopMinBase(
        row, cmPrev);
    if (cm != cmPrev)
      lstCM.push_back(cm);

    cmPrev = cm;
  }
  LOG4CXX_INFO(_logger,
               "GETCOPMINBASEHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                     << " (" << stopCPU() << ")");
  res.freeResult();

  // Get TktgCxrExcepts
  QueryGetCopTktgCxrExcpts SQLTktgCxrExcpts(_dbAdapt);
  std::vector<tse::CopMinimum*>::iterator i;
  for (i = lstCM.begin(); i != lstCM.end(); i++)
  {
    SQLTktgCxrExcpts.getTktgCxrExcpts(*i);
  }
} // QueryGetCopMinBaseHistorical::findCopMinimum()

///////////////////////////////////////////////////////////
//  QueryGetAllCopMinBase
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllCopMinBase::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllCopMinBase"));
std::string QueryGetAllCopMinBase::_baseSQL;
bool QueryGetAllCopMinBase::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllCopMinBase> g_GetAllCopMinBase;

const char*
QueryGetAllCopMinBase::getQueryName() const
{
  return "GETALLCOPMINBASE";
};

void
QueryGetAllCopMinBase::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllCopMinBaseSQLStatement<QueryGetAllCopMinBase> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLCOPMINBASE");
    substTableDef(&_baseSQL);

    QueryGetCopTktgCxrExcpts::initialize();

    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllCopMinBase::findAllCopMinimum(std::vector<tse::CopMinimum*>& lstCM)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::CopMinimum* cm = nullptr;
  tse::CopMinimum* cmPrev = nullptr;
  while ((row = res.nextRow()))
  {
    cm = QueryGetAllCopMinBaseSQLStatement<QueryGetAllCopMinBase>::mapRowToCopMinBase(row, cmPrev);
    if (cm != cmPrev)
      lstCM.push_back(cm);

    cmPrev = cm;
  }
  LOG4CXX_INFO(_logger,
               "GETALLCOPMINBASE: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                              << stopCPU() << ")");
  res.freeResult();

  // Get TktgCxrExcepts
  QueryGetCopTktgCxrExcpts SQLTktgCxrExcpts(_dbAdapt);
  std::vector<tse::CopMinimum*>::iterator i;
  for (i = lstCM.begin(); i != lstCM.end(); i++)
  {
    SQLTktgCxrExcpts.getTktgCxrExcpts(*i);
  }
} // QueryGetAllCopMinBase::findAllCopMinimum()

///////////////////////////////////////////////////////////
//  QueryGetAllCopMinBaseHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllCopMinBaseHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllCopMinBaseHistorical"));
std::string QueryGetAllCopMinBaseHistorical::_baseSQL;
bool QueryGetAllCopMinBaseHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllCopMinBaseHistorical> g_GetAllCopMinBaseHistorical;

const char*
QueryGetAllCopMinBaseHistorical::getQueryName() const
{
  return "GETALLCOPMINBASEHISTORICAL";
};

void
QueryGetAllCopMinBaseHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllCopMinBaseHistoricalSQLStatement<QueryGetAllCopMinBaseHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLCOPMINBASEHISTORICAL");
    substTableDef(&_baseSQL);

    QueryGetCopTktgCxrExcpts::initialize();

    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllCopMinBaseHistorical::findAllCopMinimum(std::vector<tse::CopMinimum*>& lstCM)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::CopMinimum* cm = nullptr;
  tse::CopMinimum* cmPrev = nullptr;
  while ((row = res.nextRow()))
  {
    cm = QueryGetAllCopMinBaseHistoricalSQLStatement<
        QueryGetAllCopMinBaseHistorical>::mapRowToCopMinBase(row, cmPrev);
    if (cm != cmPrev)
      lstCM.push_back(cm);

    cmPrev = cm;
  }
  LOG4CXX_INFO(_logger,
               "GETALLCOPMINBASEHISTORICAL: NumRows = " << res.numRows() << " Time = "
                                                        << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();

  // Get TktgCxrExcepts
  QueryGetCopTktgCxrExcpts SQLTktgCxrExcpts(_dbAdapt);
  std::vector<tse::CopMinimum*>::iterator i;
  for (i = lstCM.begin(); i != lstCM.end(); i++)
  {
    SQLTktgCxrExcpts.getTktgCxrExcpts(*i);
  }
} // QueryGetAllCurSelBaseHistorical::findAllCurrencySelection()

///////////////////////////////////////////////////////////
//  QueryGetCopTktgCxrExcpts
///////////////////////////////////////////////////////////
const char*
QueryGetCopTktgCxrExcpts::getQueryName() const
{
  return "GETCOPTKTGCXREXCPTS";
}

void
QueryGetCopTktgCxrExcpts::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCopTktgCxrExcptsSQLStatement<QueryGetCopTktgCxrExcpts> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCOPTKTGCXREXCPTS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetCopTktgCxrExcpts::getTktgCxrExcpts(CopMinimum* a_pCopMinimum)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(a_pCopMinimum->copNation(), 1);
  substParm(2, a_pCopMinimum->versionDate());
  substParm(3, a_pCopMinimum->seqNo());
  substParm(4, a_pCopMinimum->createDate());
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    a_pCopMinimum->tktgCxrExcpts().push_back(
        QueryGetCopTktgCxrExcptsSQLStatement<QueryGetCopTktgCxrExcpts>::mapRowToTktgCarrier(row));
  }
  LOG4CXX_INFO(_logger,
               "GETCOPTKTGCXREXCPTS: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                 << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetCopTktgCxrExcpts::getTktgCxrExcpts()
}
