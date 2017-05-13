//----------------------------------------------------------------------------
//  File:           QueryGetGlobalDirs.cpp
//  Description:    QueryGetGlobalDirs
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

#include "DBAccess/Queries/QueryGetGlobalDirs.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetGlobalDirsSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetGlobalDirs::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetGlobalDirs"));
std::string QueryGetGlobalDirs::_baseSQL;
bool QueryGetGlobalDirs::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetGlobalDirs> g_GetGlobalDirs;

const char*
QueryGetGlobalDirs::getQueryName() const
{
  return "GETGLOBALDIRS";
}

void
QueryGetGlobalDirs::initialize()
{
  if (!_isInitialized)
  {
    QueryGetGlobalDirsSQLStatement<QueryGetGlobalDirs> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETGLOBALDIRS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetGlobalDirs::findAllGlobalDir(std::vector<tse::GlobalDir*>& globalDirs)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  std::string prev("");
  DateTime past;
  tse::GlobalDir* globalDir(nullptr);
  substCurrentDate();
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    std::string curr =
        row->getString(QueryGetGlobalDirsSQLStatement<QueryGetGlobalDirs>::GLOBALDIR);
    DateTime now = row->getDate(QueryGetGlobalDirsSQLStatement<QueryGetGlobalDirs>::CREATEDATE);
    if (curr != prev || past != now)
    {
      if (globalDir != nullptr)
      {
        globalDirs.push_back(globalDir);
      }
      globalDir = QueryGetGlobalDirsSQLStatement<QueryGetGlobalDirs>::mapRowToGlobalDir(row);
      prev = curr;
      past = now;
    }
    std::string directionality =
        row->getString(QueryGetGlobalDirsSQLStatement<QueryGetGlobalDirs>::DIRECTIONALITY);
    if (directionality != "")
    {
      tse::GlobalDirSeg* globalDirSeg =
          QueryGetGlobalDirsSQLStatement<QueryGetGlobalDirs>::mapRowToGlobalDirSeg(row);
      globalDir->segs().push_back(*globalDirSeg);
      delete globalDirSeg;
    }
  }
  if (globalDir != nullptr)
  {
    globalDirs.push_back(globalDir);
  }
  LOG4CXX_INFO(_logger,
               "GETGLOBALDIRS: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                           << stopCPU() << ")");
  res.freeResult();
} // findAllGlobalDir()
///////////////////////////////////////////////////////////
//
//  QueryGetGlobalDirsHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetGlobalDirsHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetGlobalDirsHistorical"));
std::string QueryGetGlobalDirsHistorical::_baseSQL;
bool QueryGetGlobalDirsHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetGlobalDirsHistorical> g_GetGlobalDirsHistorical;

const char*
QueryGetGlobalDirsHistorical::getQueryName() const
{
  return "GETGLOBALDIRSHISTORICAL";
}

void
QueryGetGlobalDirsHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetGlobalDirsHistoricalSQLStatement<QueryGetGlobalDirsHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETGLOBALDIRSHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetGlobalDirsHistorical::findAllGlobalDir(std::vector<tse::GlobalDir*>& globalDirs)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  std::string prev("");
  DateTime past;
  tse::GlobalDir* globalDir(nullptr);
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    std::string curr = row->getString(
        QueryGetGlobalDirsHistoricalSQLStatement<QueryGetGlobalDirsHistorical>::GLOBALDIR);
    DateTime now = row->getDate(
        QueryGetGlobalDirsHistoricalSQLStatement<QueryGetGlobalDirsHistorical>::CREATEDATE);
    if (curr != prev || past != now)
    {
      if (globalDir != nullptr)
      {
        globalDirs.push_back(globalDir);
      }
      globalDir =
          QueryGetGlobalDirsHistoricalSQLStatement<QueryGetGlobalDirsHistorical>::mapRowToGlobalDir(
              row);
      prev = curr;
      past = now;
    }
    std::string directionality = row->getString(
        QueryGetGlobalDirsHistoricalSQLStatement<QueryGetGlobalDirsHistorical>::DIRECTIONALITY);
    if (directionality != "")
    {
      tse::GlobalDirSeg* globalDirSeg = QueryGetGlobalDirsHistoricalSQLStatement<
          QueryGetGlobalDirsHistorical>::mapRowToGlobalDirSeg(row);
      globalDir->segs().push_back(*globalDirSeg);
      delete globalDirSeg;
    }
  }
  if (globalDir != nullptr)
  {
    globalDirs.push_back(globalDir);
  }
  LOG4CXX_INFO(_logger,
               "GETGLOBALDIRSHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                     << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllGlobalDir()
///////////////////////////////////////////////////////////
//
//  QueryGetAllGlobalDirsHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllGlobalDirsHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllGlobalDirsHistorical"));
std::string QueryGetAllGlobalDirsHistorical::_baseSQL;
bool QueryGetAllGlobalDirsHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllGlobalDirsHistorical> g_GetAllGlobalDirsHistorical;

const char*
QueryGetAllGlobalDirsHistorical::getQueryName() const
{
  return "GETALLGLOBALDIRSHISTORICAL";
}

void
QueryGetAllGlobalDirsHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllGlobalDirsHistoricalSQLStatement<QueryGetAllGlobalDirsHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLGLOBALDIRSHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllGlobalDirsHistorical::findAllGlobalDir(std::vector<tse::GlobalDir*>& globalDirs)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  std::string prev("");
  DateTime past;
  tse::GlobalDir* globalDir(nullptr);
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    std::string curr = row->getString(
        QueryGetAllGlobalDirsHistoricalSQLStatement<QueryGetAllGlobalDirsHistorical>::GLOBALDIR);
    DateTime now = row->getDate(
        QueryGetAllGlobalDirsHistoricalSQLStatement<QueryGetAllGlobalDirsHistorical>::CREATEDATE);
    if (curr != prev || past != now)
    {
      if (globalDir != nullptr)
      {
        globalDirs.push_back(globalDir);
      }
      globalDir = QueryGetAllGlobalDirsHistoricalSQLStatement<
          QueryGetAllGlobalDirsHistorical>::mapRowToGlobalDir(row);
      prev = curr;
      past = now;
    }
    std::string directionality = row->getString(QueryGetAllGlobalDirsHistoricalSQLStatement<
        QueryGetAllGlobalDirsHistorical>::DIRECTIONALITY);
    if (directionality != "")
    {
      tse::GlobalDirSeg* globalDirSeg = QueryGetAllGlobalDirsHistoricalSQLStatement<
          QueryGetAllGlobalDirsHistorical>::mapRowToGlobalDirSeg(row);
      globalDir->segs().push_back(*globalDirSeg);
      delete globalDirSeg;
    }
  }
  if (globalDir != nullptr)
  {
    globalDirs.push_back(globalDir);
  }
  LOG4CXX_INFO(_logger,
               "GETALLGLOBALDIRSHISTORICAL: NumRows = " << res.numRows() << " Time = "
                                                        << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllGlobalDir()
}
