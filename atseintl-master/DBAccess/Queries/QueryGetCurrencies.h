//----------------------------------------------------------------------------
//          File:           QueryGetCurrencies.h
//          Description:    QueryGetCurrencies
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
#include "DBAccess/Currency.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetCurrencies : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCurrencies(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetCurrencies(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetCurrencies() {};

  virtual const char* getQueryName() const override;

  void findCurrency(std::vector<tse::Currency*>& lstCurr, const CurrencyCode& cur);

  static void initialize();

  const QueryGetCurrencies& operator=(const QueryGetCurrencies& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetCurrencies& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  };

protected:
private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetCurrencies

class QueryGetCurrenciesHistorical : public tse::QueryGetCurrencies
{
public:
  QueryGetCurrenciesHistorical(DBAdapter* dbAdapt) : QueryGetCurrencies(dbAdapt, _baseSQL) {};
  virtual ~QueryGetCurrenciesHistorical() {};
  virtual const char* getQueryName() const override;

  void findCurrency(std::vector<tse::Currency*>& lstCurr, const CurrencyCode& cur);

  static void initialize();

protected:
private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetCurrenciesHistorical

class QueryGetAllCurrencies : public QueryGetCurrencies
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllCurrencies(DBAdapter* dbAdapt) : QueryGetCurrencies(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllCurrencies() {};
  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::Currency*>& lstCurr) { findAllCurrency(lstCurr); }

  void findAllCurrency(std::vector<tse::Currency*>& lstCurr);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllCurrencies

class QueryGetAllCurrenciesHistorical : public QueryGetCurrencies
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllCurrenciesHistorical(DBAdapter* dbAdapt) : QueryGetCurrencies(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllCurrenciesHistorical() {};
  virtual const char* getQueryName() const override;

  void findAllCurrency(std::vector<tse::Currency*>& lstCurr);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllCurrenciesHistorical
} // namespace tse

