//----------------------------------------------------------------------------
//          File:           QueryGetFareClassApp.h
//          Description:    QueryGetFareClassApp
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
#include "DBAccess/FareClassAppInfo.h"
#include "DBAccess/FareClassAppSegInfo.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{

class QueryGetFareClassApp : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareClassApp(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetFareClassApp(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetFareClassApp() {};
  virtual const char* getQueryName() const override;

  void findFareClassApp(std::vector<const tse::FareClassAppInfo*>& lstFCA,
                        const VendorCode& vendor,
                        const CarrierCode& carrier,
                        const TariffNumber ruleTariff,
                        const RuleNumber& rule,
                        const FareClassCode& fareClass);

  static void initialize();

  const QueryGetFareClassApp& operator=(const QueryGetFareClassApp& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetFareClassApp& operator=(const std::string& Another)
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
}; // class QueryGetFareClassApp

class QueryGetFareClassAppHistorical : public QueryGetFareClassApp
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareClassAppHistorical(DBAdapter* dbAdapt) : QueryGetFareClassApp(dbAdapt, _baseSQL) {};
  virtual ~QueryGetFareClassAppHistorical() {};
  virtual const char* getQueryName() const override;

  void findFareClassApp(std::vector<const tse::FareClassAppInfo*>& lstFCA,
                        const VendorCode& vendor,
                        const CarrierCode& carrier,
                        const TariffNumber ruleTariff,
                        const RuleNumber& rule,
                        const FareClassCode& fareClass,
                        const DateTime& startDate,
                        const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetFareClassAppHistorical

class QueryGetAllFareClassApp : public QueryGetFareClassApp
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllFareClassApp(DBAdapter* dbAdapt) : QueryGetFareClassApp(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllFareClassApp() {};
  virtual const char* getQueryName() const override;

  void execute(std::vector<const tse::FareClassAppInfo*>& lstFCA) { findAllFareClassApp(lstFCA); }

  void findAllFareClassApp(std::vector<const tse::FareClassAppInfo*>& lstFCA);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllFareClassApp
} // namespace tse

