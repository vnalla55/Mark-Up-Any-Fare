//----------------------------------------------------------------------------
//          File:           QueryGetRouting.h
//          Description:    QueryGetRouting
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
#include "DBAccess/Routing.h"
#include "DBAccess/RoutingMap.h"
#include "DBAccess/RoutingRestriction.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{

class QueryGetRouting : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetRouting(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetRouting(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetRouting() {}
  const char* getQueryName() const override;

  void findRouting(std::vector<tse::Routing*>& routings,
                   const VendorCode& vendor,
                   const CarrierCode& carrier,
                   int& routingTariff,
                   const RoutingNumber& routing);

  static void initialize();

  const QueryGetRouting& operator=(const QueryGetRouting& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetRouting& operator=(const std::string& Another)
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
}; // class QueryGetRouting

class QueryGetRoutingHistorical : public QueryGetRouting
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetRoutingHistorical(DBAdapter* dbAdapt) : QueryGetRouting(dbAdapt, _baseSQL) {};
  virtual ~QueryGetRoutingHistorical() {}
  const char* getQueryName() const override;

  void findRouting(std::vector<tse::Routing*>& routings,
                   const VendorCode& vendor,
                   const CarrierCode& carrier,
                   int& routingTariff,
                   const RoutingNumber& routing,
                   const DateTime& startDate,
                   const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetRoutingHistorical

class QueryGetRoutingRest : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetRoutingRest(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetRoutingRest(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetRoutingRest() {}
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  void getRoutingRest(Routing* a_pRouting);

  static void initialize();

  const QueryGetRoutingRest& operator=(const QueryGetRoutingRest& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetRoutingRest& operator=(const std::string& Another)
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
}; // class QueryGetRoutingRest

class QueryGetRoutingRestHistorical : public QueryGetRoutingRest
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetRoutingRestHistorical(DBAdapter* dbAdapt) : QueryGetRoutingRest(dbAdapt, _baseSQL) {};
  virtual ~QueryGetRoutingRestHistorical() {}
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  void getRoutingRest(Routing* a_pRouting);

  static void initialize();

  const QueryGetRoutingRestHistorical& operator=(const QueryGetRoutingRestHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetRoutingRestHistorical& operator=(const std::string& Another)
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
}; // class QueryGetRoutingRestHist
} // namespace tse
