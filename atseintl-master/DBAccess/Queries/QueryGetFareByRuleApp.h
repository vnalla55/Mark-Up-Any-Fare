//----------------------------------------------------------------------------
//          File:           QueryGetFareByRuleApp.h
//          Description:    QueryGetFareByRuleApp
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
#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{

class QueryGetFareByRuleApp : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareByRuleApp(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetFareByRuleApp(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetFareByRuleApp() {};
  virtual const char* getQueryName() const override;

  void findFareByRuleApp(std::vector<tse::FareByRuleApp*>& fbrApps,
                         const CarrierCode& carrier,
                         const AccountCode& accountCode);

  static void initialize();

  const QueryGetFareByRuleApp& operator=(const QueryGetFareByRuleApp& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetFareByRuleApp& operator=(const std::string& Another)
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
}; // class QueryGetFareByRuleApp

class QueryGetFareByRuleAppHistorical : public QueryGetFareByRuleApp
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareByRuleAppHistorical(DBAdapter* dbAdapt) : QueryGetFareByRuleApp(dbAdapt, _baseSQL) {};
  virtual ~QueryGetFareByRuleAppHistorical() {};
  virtual const char* getQueryName() const override;

  void findFareByRuleApp(std::vector<tse::FareByRuleApp*>& fbrApps,
                         const CarrierCode& carrier,
                         const AccountCode& accountCode,
                         const DateTime& startDate,
                         const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetFareByRuleAppHist

class QueryGetFareByRuleAppRuleTariff : public QueryGetFareByRuleApp
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareByRuleAppRuleTariff(DBAdapter* dbAdapt) : QueryGetFareByRuleApp(dbAdapt, _baseSQL) {};
  virtual ~QueryGetFareByRuleAppRuleTariff() {};
  virtual const char* getQueryName() const override;

  void findFareByRuleApp(std::vector<tse::FareByRuleApp*>& fbrApps,
                         const CarrierCode& carrier,
                         const TariffNumber& ruleTariff);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetFareByRuleAppRuleTariff : public QueryGetFareByRuleApp

class QueryGetFareByRuleAppRuleTariffHistorical : public QueryGetFareByRuleApp
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareByRuleAppRuleTariffHistorical(DBAdapter* dbAdapt)
    : QueryGetFareByRuleApp(dbAdapt, _baseSQL) {};
  virtual ~QueryGetFareByRuleAppRuleTariffHistorical() {};
  virtual const char* getQueryName() const override;

  void findFareByRuleApp(std::vector<tse::FareByRuleApp*>& fbrApps,
                         const CarrierCode& carrier,
                         const TariffNumber& ruleTariff,
                         const DateTime& startDate,
                         const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;

  void setAdapterAndBaseSQL(DBAdapter* dbAdapt);
}; // class QueryGetFareByRuleAppRuleTariffHist
} // namespace tse
