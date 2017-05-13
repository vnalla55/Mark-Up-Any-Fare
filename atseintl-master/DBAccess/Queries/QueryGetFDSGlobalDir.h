//----------------------------------------------------------------------------
//          File:           QueryGetFDSGlobalDir.h
//          Description:    QueryGetFDSGlobalDir
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
#include "DBAccess/FDSGlobalDir.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetFDSGlobalDir : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFDSGlobalDir(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetFDSGlobalDir(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetFDSGlobalDir() {};

  virtual const char* getQueryName() const override;

  void findFDSGlobalDir(std::vector<tse::FDSGlobalDir*>& infos,
                        const Indicator& userApplType,
                        const UserApplCode& userAppl,
                        const Indicator& pseudoCityType,
                        const PseudoCityCode& pseudoCity,
                        const TJRGroup& tjrGroup);

  static void initialize();

  const QueryGetFDSGlobalDir& operator=(const QueryGetFDSGlobalDir& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetFDSGlobalDir& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  };

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetFDSGlobalDir

class QueryGetAllFDSGlobalDir : public QueryGetFDSGlobalDir
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllFDSGlobalDir(DBAdapter* dbAdapt) : QueryGetFDSGlobalDir(dbAdapt, _baseSQL) {};

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::FDSGlobalDir*>& infos) { findAllFDSGlobalDir(infos); }

  void findAllFDSGlobalDir(std::vector<tse::FDSGlobalDir*>& infos);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllFDSGlobalDir
} // namespace tse

