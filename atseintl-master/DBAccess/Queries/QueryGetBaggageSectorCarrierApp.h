//----------------------------------------------------------------------------
//     � 2010, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "Common/Logger.h"
#include "DBAccess/BaggageSectorCarrierApp.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetBaggageSectorCarrierApp : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetBaggageSectorCarrierApp(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetBaggageSectorCarrierApp(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetBaggageSectorCarrierApp() {};

  virtual const char* getQueryName() const override;

  void findBaggageSectorCarrierApp(std::vector<tse::BaggageSectorCarrierApp*>& crxApp,
                                   const CarrierCode& carrier);

  static void initialize();

  const QueryGetBaggageSectorCarrierApp& operator=(const QueryGetBaggageSectorCarrierApp& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetBaggageSectorCarrierApp& operator=(const std::string& Another)
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
};
class QueryGetBaggageSectorCarrierAppHistorical : public tse::QueryGetBaggageSectorCarrierApp
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetBaggageSectorCarrierAppHistorical(DBAdapter* dbAdapt)
    : QueryGetBaggageSectorCarrierApp(dbAdapt, _baseSQL) {};
  virtual ~QueryGetBaggageSectorCarrierAppHistorical() {};
  virtual const char* getQueryName() const override;

  void findBaggageSectorCarrierApp(std::vector<tse::BaggageSectorCarrierApp*>& crxApp,
                                   const CarrierCode& carrier);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

class QueryGetAllBaggageSectorCarrierApp : public QueryGetBaggageSectorCarrierApp
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllBaggageSectorCarrierApp(DBAdapter* dbAdapt)
    : QueryGetBaggageSectorCarrierApp(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllBaggageSectorCarrierApp() {};
  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::BaggageSectorCarrierApp*>& crxApp)
  {
    findAllBaggageSectorCarrierApp(crxApp);
  }

  void findAllBaggageSectorCarrierApp(std::vector<tse::BaggageSectorCarrierApp*>& crxApp);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

class QueryGetAllBaggageSectorCarrierAppHistorical : public QueryGetBaggageSectorCarrierApp
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllBaggageSectorCarrierAppHistorical(DBAdapter* dbAdapt)
    : QueryGetBaggageSectorCarrierApp(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllBaggageSectorCarrierAppHistorical() {};
  virtual const char* getQueryName() const override;

  void findAllBaggageSectorCarrierApp(std::vector<tse::BaggageSectorCarrierApp*>& crxApp);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
} // namespace tse

