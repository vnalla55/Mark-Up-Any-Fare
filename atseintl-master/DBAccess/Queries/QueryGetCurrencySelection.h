//----------------------------------------------------------------------------
//          File:           QueryGetCurrencySelection.h
//          Description:    QueryGetCurrencySelection
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
#include "DBAccess/CurrencySelection.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetCurSelRestrCur : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCurSelRestrCur(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  virtual ~QueryGetCurSelRestrCur() {};

  void resetSQL()
  {
    *this = _baseSQL;
  };

  const char* getQueryName() const override;

  void getRestrCurs(CurrencySelection* a_pCurSel);

  static void initialize();

  const QueryGetCurSelRestrCur& operator=(const QueryGetCurSelRestrCur& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetCurSelRestrCur& operator=(const std::string& Another)
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
}; // class QueryGetCurSelRestrCur

class QueryGetCurSelPsgrType : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCurSelPsgrType(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  virtual ~QueryGetCurSelPsgrType() {};

  void resetSQL()
  {
    *this = _baseSQL;
  };

  const char* getQueryName() const override;

  void getPsgrTypes(CurrencySelection* a_pCurSel);

  static void initialize();

  const QueryGetCurSelPsgrType& operator=(const QueryGetCurSelPsgrType& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetCurSelPsgrType& operator=(const std::string& Another)
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
}; // class QueryGetCurSelPsgrType

class QueryGetCurSelAseanCur : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCurSelAseanCur(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  virtual ~QueryGetCurSelAseanCur() {};

  void resetSQL()
  {
    *this = _baseSQL;
  };

  const char* getQueryName() const override;

  void getAseanCurs(CurrencySelection* a_pCurSel);

  static void initialize();

  const QueryGetCurSelAseanCur& operator=(const QueryGetCurSelAseanCur& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetCurSelAseanCur& operator=(const std::string& Another)
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
}; // class QueryGetCurSelAseanCur

class QueryGetCurSelTextMsg : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCurSelTextMsg(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  virtual ~QueryGetCurSelTextMsg() {};

  void resetSQL()
  {
    *this = _baseSQL;
  };

  const char* getQueryName() const override;

  void getTextMsg(CurrencySelection* a_pCurSel);

  static void initialize();

  const QueryGetCurSelTextMsg& operator=(const QueryGetCurSelTextMsg& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetCurSelTextMsg& operator=(const std::string& Another)
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
}; // class QueryGetCurSelTextMsg

class QueryGetOneCurSelBase : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetOneCurSelBase(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetOneCurSelBase(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetOneCurSelBase() {};

  virtual const char* getQueryName() const override;

  void findCurrencySelection(std::vector<tse::CurrencySelection*>& lstCS, NationCode& nation);

  static void initialize();

  const QueryGetOneCurSelBase& operator=(const QueryGetOneCurSelBase& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetOneCurSelBase& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  };

protected:
  void getCurSelChildren(std::vector<tse::CurrencySelection*>& lstCS);

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetOneCurSelBase

class QueryGetOneCurSelBaseHistorical : public tse::QueryGetOneCurSelBase
{
public:
  QueryGetOneCurSelBaseHistorical(DBAdapter* dbAdapt) : QueryGetOneCurSelBase(dbAdapt, _baseSQL) {}
  virtual ~QueryGetOneCurSelBaseHistorical() {};
  virtual const char* getQueryName() const override;

  void findCurrencySelection(std::vector<tse::CurrencySelection*>& lstCS, NationCode& nation);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetOneCurSelBaseHistorical

class QueryGetAllCurSelBase : public QueryGetOneCurSelBase
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllCurSelBase(DBAdapter* dbAdapt) : QueryGetOneCurSelBase(dbAdapt, _baseSQL) {};
  virtual const char* getQueryName() const override;
  void execute(std::vector<tse::CurrencySelection*>& lstCS) { findAllCurrencySelection(lstCS); }

  void findAllCurrencySelection(std::vector<tse::CurrencySelection*>& lstCS);
  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllCurSelBase

class QueryGetAllCurSelBaseHistorical : public QueryGetOneCurSelBase
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllCurSelBaseHistorical(DBAdapter* dbAdapt) : QueryGetOneCurSelBase(dbAdapt, _baseSQL) {};
  virtual const char* getQueryName() const override;
  void findAllCurrencySelection(std::vector<tse::CurrencySelection*>& lstCS);
  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllCurSelBaseHistorical
} // namespace tse

