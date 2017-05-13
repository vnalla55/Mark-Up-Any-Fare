//----------------------------------------------------------------------------
//          File:           QueryGetNoPNROptions.h
//          Description:    QueryGetNoPNROptions
//          Created:        1/11/2008
// Authors:         Karolina Golebiewska
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
#include "DBAccess/NoPNROptions.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetNoPNROptions : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetNoPNROptions(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetNoPNROptions(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetNoPNROptions() {};

  virtual const char* getQueryName() const override;

  void findNoPNROptions(std::vector<tse::NoPNROptions*>& npos,
                        const Indicator userApplType,
                        const UserApplCode& userAppl);

  static void initialize();

  const QueryGetNoPNROptions& operator=(const QueryGetNoPNROptions& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetNoPNROptions& operator=(const std::string& Another)
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
}; // class QueryGetNoPNROptions

class QueryGetAllNoPNROptions : public QueryGetNoPNROptions
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllNoPNROptions(DBAdapter* dbAdapt) : QueryGetNoPNROptions(dbAdapt, _baseSQL) {};

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::NoPNROptions*>& npos) { findAllNoPNROptions(npos); }

  void findAllNoPNROptions(std::vector<tse::NoPNROptions*>& npos);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllNoPNROptions
} // namespace tse

