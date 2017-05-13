//----------------------------------------------------------------------------
//          File:           QueryGetNation.h
//          Description:    QueryGetNation
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
#include "DBAccess/Nation.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetNation : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetNation(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetNation(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetNation() {};

  virtual const char* getQueryName() const override;

  void findNation(std::vector<tse::Nation*>& nations, const NationCode& nation);

  static void initialize();

  const QueryGetNation& operator=(const QueryGetNation& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetNation& operator=(const std::string& Another)
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
}; // class QueryGetNation

class QueryGetNations : public QueryGetNation
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetNations(DBAdapter* dbAdapt) : QueryGetNation(dbAdapt, _baseSQL) {};

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::Nation*>& nations) { findAllNations(nations); }

  void findAllNations(std::vector<tse::Nation*>& nations);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetNations

class QueryGetNationHistorical : public QueryGetNation
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetNationHistorical(DBAdapter* dbAdapt) : QueryGetNation(dbAdapt, _baseSQL) {}
  QueryGetNationHistorical(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : QueryGetNation(dbAdapt, sqlStatement)
  {
  }

  virtual ~QueryGetNationHistorical() {};

  virtual const char* getQueryName() const override;

  void findNation(std::vector<tse::Nation*>& nations, const NationCode& nation);

  static void initialize();

  const QueryGetNationHistorical& operator=(const QueryGetNationHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetNationHistorical& operator=(const std::string& Another)
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
}; // class QueryGetNation

class QueryGetNationsHistorical : public QueryGetNationHistorical
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetNationsHistorical(DBAdapter* dbAdapt) : QueryGetNationHistorical(dbAdapt, _baseSQL) {}

  virtual const char* getQueryName() const override;

  void findAllNations(std::vector<tse::Nation*>& nations);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetNations

} // namespace tse

