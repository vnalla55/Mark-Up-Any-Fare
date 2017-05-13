//----------------------------------------------------------------------------
//          File:           QueryGetGlobalDirs.h
//          Description:    QueryGetGlobalDirs
//          Created:        3/2/2006
// Authors:         Mike Lillis
//
//          Updates:
//
//     � 2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "Common/Logger.h"
#include "DBAccess/GlobalDir.h"
#include "DBAccess/GlobalDirSeg.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{

class Row;

class QueryGetGlobalDirs : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetGlobalDirs(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetGlobalDirs(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetGlobalDirs() {};

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::GlobalDir*>& globalDirs) { findAllGlobalDir(globalDirs); }

  void findAllGlobalDir(std::vector<tse::GlobalDir*>& globalDirs);

  static void initialize();

  const QueryGetGlobalDirs& operator=(const QueryGetGlobalDirs& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetGlobalDirs& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  };

protected:
  tse::GlobalDir* mapRowToGlobalDir(Row* row);
  tse::GlobalDirSeg* mapRowToGlobalDirSeg(Row* row);

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetGlobalDirs

class QueryGetGlobalDirsHistorical : public tse::QueryGetGlobalDirs
{
public:
  QueryGetGlobalDirsHistorical(DBAdapter* dbAdapt) : QueryGetGlobalDirs(dbAdapt, _baseSQL) {};
  virtual ~QueryGetGlobalDirsHistorical() {};
  virtual const char* getQueryName() const override;

  void findAllGlobalDir(std::vector<tse::GlobalDir*>& globalDirs);

  static void initialize();

protected:
  tse::GlobalDir* mapRowToGlobalDir(Row* row);
  tse::GlobalDirSeg* mapRowToGlobalDirSeg(Row* row);

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetGlobalDirsHistorical

class QueryGetAllGlobalDirsHistorical : public tse::QueryGetGlobalDirs
{
public:
  QueryGetAllGlobalDirsHistorical(DBAdapter* dbAdapt) : QueryGetGlobalDirs(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllGlobalDirsHistorical() {};
  virtual const char* getQueryName() const override;

  void findAllGlobalDir(std::vector<tse::GlobalDir*>& globalDirs);

  static void initialize();

protected:
  tse::GlobalDir* mapRowToGlobalDir(Row* row);
  tse::GlobalDirSeg* mapRowToGlobalDirSeg(Row* row);

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetGlobalDirsHistorical
} // namespace tse

