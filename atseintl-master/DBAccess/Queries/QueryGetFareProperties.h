//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "DBAccess/SQLQuery.h"

namespace tse
{
class FareProperties;

class QueryGetFareProperties : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareProperties(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetFareProperties(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetFareProperties() {}

  virtual const char* getQueryName() const override;

  void findFareProperties(std::vector<FareProperties*>& fareProperties,
                          const VendorCode& vendor,
                          const CarrierCode& carrier,
                          const TariffNumber& tariff,
                          const RuleNumber& rule);

  static void initialize();

  const QueryGetFareProperties& operator=(const QueryGetFareProperties& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }

  const QueryGetFareProperties& operator=(const std::string& Another)
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

class QueryGetFarePropertiesHistorical : public QueryGetFareProperties
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFarePropertiesHistorical(DBAdapter* dbAdapt) : QueryGetFareProperties(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetFarePropertiesHistorical() {}
  virtual const char* getQueryName() const override;

  void findFareProperties(std::vector<FareProperties*>& fareProperties,
                          const VendorCode& vendor,
                          const CarrierCode& carrier,
                          const TariffNumber& tariff,
                          const RuleNumber& rule,
                          const DateTime& startDate,
                          const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
} // tse

