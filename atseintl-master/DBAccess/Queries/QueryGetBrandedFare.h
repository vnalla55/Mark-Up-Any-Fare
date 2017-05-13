//----------------------------------------------------------------------------
//          File:           QueryGetBrandedFare.h
//          Description:    QueryGetBrandedFare
//          Created:        03/22/2013
//
//     Copyright 2013, Sabre Inc. All rights reserved. This software/documentation is
//     confidential and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "Common/Logger.h"
#include "DBAccess/BrandedFare.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{
class QueryGetBrandedFare : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetBrandedFare(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetBrandedFare(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetBrandedFare() {}
  virtual const char* getQueryName() const override;

  void findBrandedFare(std::vector<BrandedFare*>& lst,
                       const VendorCode& vendor,
                       const CarrierCode& carrier);

  static void initialize();

  const QueryGetBrandedFare& operator=(const QueryGetBrandedFare& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetBrandedFare& operator=(const std::string& Another)
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
};

class QueryGetBrandedFareHistorical : public QueryGetBrandedFare
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetBrandedFareHistorical(DBAdapter* dbAdapt) : QueryGetBrandedFare(dbAdapt, _baseSQL) {}
  virtual ~QueryGetBrandedFareHistorical() {}
  virtual const char* getQueryName() const override;

  void findBrandedFare(std::vector<BrandedFare*>& lst,
                       const VendorCode& vendor,
                       const CarrierCode& carrier,
                       const DateTime& startDate,
                       const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

class QueryGetAllBrandedFare : public QueryGetBrandedFare
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllBrandedFare(DBAdapter* dbAdapt) : QueryGetBrandedFare(dbAdapt, _baseSQL) {}
  virtual ~QueryGetAllBrandedFare() {}
  virtual const char* getQueryName() const override;

  void execute(std::vector<BrandedFare*>& lst) { findAllBrandedFare(lst); }
  void findAllBrandedFare(std::vector<BrandedFare*>& lst);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
} // tse

