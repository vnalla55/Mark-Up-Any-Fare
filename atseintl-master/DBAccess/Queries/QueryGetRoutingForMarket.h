//----------------------------------------------------------------------------
//          File:           QueryGetRoutingForMarket.h
//          Description:    QueryGetRoutingForMarket
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
#include "DBAccess/RoutingKeyInfo.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{

class QueryGetRoutingForDomMarket : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetRoutingForDomMarket(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetRoutingForDomMarket(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetRoutingForDomMarket() {}
  virtual const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  }; // If have to load domMkts (1st time)

  void findRoutingForMarket(std::vector<tse::RoutingKeyInfo*>& infos,
                            LocCode& market1,
                            LocCode& market2,
                            CarrierCode& carrier);

  void getDomRoutings(std::vector<tse::RoutingKeyInfo*>& infos,
                      LocCode& market1,
                      LocCode& market2,
                      CarrierCode& carrier);

  static void initialize();

  const QueryGetRoutingForDomMarket& operator=(const QueryGetRoutingForDomMarket& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetRoutingForDomMarket& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  }

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;

protected:
  static boost::mutex _mutex; // Thread Safety
  static std::vector<LocCode> domMkts;
}; // class QueryGetRoutingForDomMarket

class QueryGetRoutingForDomMarketHistorical : public QueryGetRoutingForDomMarket
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetRoutingForDomMarketHistorical(DBAdapter* dbAdapt)
    : QueryGetRoutingForDomMarket(dbAdapt, _baseSQL) {};
  virtual ~QueryGetRoutingForDomMarketHistorical() {}
  virtual const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  }; // If have to load domMkts (1st time)

  void findRoutingForMarket(std::vector<tse::RoutingKeyInfo*>& infos,
                            LocCode& market1,
                            LocCode& market2,
                            CarrierCode& carrier,
                            const DateTime& startDate,
                            const DateTime& endDate);

  void getDomRoutings(std::vector<tse::RoutingKeyInfo*>& infos,
                      LocCode& market1,
                      LocCode& market2,
                      CarrierCode& carrier,
                      const DateTime& startDate,
                      const DateTime& endDate);

  static void initialize();

  const QueryGetRoutingForDomMarketHistorical&
  operator=(const QueryGetRoutingForDomMarketHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetRoutingForDomMarketHistorical& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  }

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetRoutingForDomMarketHistorical

class QueryGetRoutingForIntlMarket : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetRoutingForIntlMarket(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetRoutingForIntlMarket(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetRoutingForIntlMarket() {}
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  void getIntlRoutings(std::vector<tse::RoutingKeyInfo*>& infos,
                       LocCode& market1,
                       LocCode& market2,
                       CarrierCode& carrier);

  static void initialize();

  const QueryGetRoutingForIntlMarket& operator=(const QueryGetRoutingForIntlMarket& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetRoutingForIntlMarket& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  }

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetRoutingForIntlMarket

class QueryGetRoutingForIntlMarketHistorical : public QueryGetRoutingForIntlMarket
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetRoutingForIntlMarketHistorical(DBAdapter* dbAdapt)
    : QueryGetRoutingForIntlMarket(dbAdapt, _baseSQL) {};
  virtual ~QueryGetRoutingForIntlMarketHistorical() {}
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  void getIntlRoutings(std::vector<tse::RoutingKeyInfo*>& infos,
                       LocCode& market1,
                       LocCode& market2,
                       CarrierCode& carrier,
                       const DateTime& startDate,
                       const DateTime& endDate);

  static void initialize();

  const QueryGetRoutingForIntlMarketHistorical&
  operator=(const QueryGetRoutingForIntlMarketHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetRoutingForIntlMarketHistorical& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  }

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetRoutingForIntlMarketHistorical

class QueryGetDomMarkets_Rtg : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetDomMarkets_Rtg(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  virtual ~QueryGetDomMarkets_Rtg() {}
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  void getDomMarkets(std::vector<LocCode> domMkts);

  static void initialize();

  const QueryGetDomMarkets_Rtg& operator=(const QueryGetDomMarkets_Rtg& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetDomMarkets_Rtg& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  }

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetDomMarkets_Rtg
} // namespace tse

