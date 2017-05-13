//----------------------------------------------------------------------------
//          File:           QueryGetCopMinimum.h
//          Description:    QueryGetCopMinimum
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
#include "DBAccess/CopMinimum.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{

class QueryGetCopTktgCxrExcpts : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCopTktgCxrExcpts(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  virtual ~QueryGetCopTktgCxrExcpts() {};

  void resetSQL()
  {
    *this = _baseSQL;
  };

  const char* getQueryName() const override;

  void getTktgCxrExcpts(CopMinimum* a_pCopMinimum);

  static void initialize();

  const QueryGetCopTktgCxrExcpts& operator=(const QueryGetCopTktgCxrExcpts& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetCopTktgCxrExcpts& operator=(const std::string& Another)
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
}; // class QueryGetCopTktgCxrExcpts

class QueryGetCopMinBase : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCopMinBase(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetCopMinBase(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetCopMinBase() {};

  virtual const char* getQueryName() const override;

  void findCopMinimum(std::vector<tse::CopMinimum*>& lstCM, const NationCode& nation);

  static void initialize();

  const QueryGetCopMinBase& operator=(const QueryGetCopMinBase& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetCopMinBase& operator=(const std::string& Another)
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
}; // class QueryGetCopMinBase

class QueryGetCopMinBaseHistorical : public tse::QueryGetCopMinBase
{
public:
  QueryGetCopMinBaseHistorical(DBAdapter* dbAdapt) : QueryGetCopMinBase(dbAdapt, _baseSQL) {}
  virtual ~QueryGetCopMinBaseHistorical() {};
  virtual const char* getQueryName() const override;

  void findCopMinimum(std::vector<tse::CopMinimum*>& lstCM, const NationCode& nation);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetCopMinBaseHistorical

class QueryGetAllCopMinBase : public QueryGetCopMinBase
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllCopMinBase(DBAdapter* dbAdapt) : QueryGetCopMinBase(dbAdapt, _baseSQL) {};
  virtual const char* getQueryName() const override;
  void execute(std::vector<tse::CopMinimum*>& lstCM) { findAllCopMinimum(lstCM); }

  void findAllCopMinimum(std::vector<tse::CopMinimum*>& lstCM);
  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllCopMinBase

class QueryGetAllCopMinBaseHistorical : public QueryGetCopMinBase
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllCopMinBaseHistorical(DBAdapter* dbAdapt) : QueryGetCopMinBase(dbAdapt, _baseSQL) {};
  virtual const char* getQueryName() const override;
  void findAllCopMinimum(std::vector<tse::CopMinimum*>& lstCM);
  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllCopMinBaseHistorical
} // namespace tse

