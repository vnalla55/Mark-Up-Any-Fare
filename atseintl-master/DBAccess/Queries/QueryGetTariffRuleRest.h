//----------------------------------------------------------------------------
//          File:           QueryGetTariffRuleRest.h
//          Description:    QueryGetTariffRuleRest
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
#include "DBAccess/SQLQuery.h"
#include "DBAccess/TariffRuleRest.h"

namespace tse
{

class QueryGetTariffRuleRest : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTariffRuleRest(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetTariffRuleRest(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetTariffRuleRest() {}
  virtual const char* getQueryName() const override;

  void findTariffRuleRest(std::vector<TariffRuleRest*>& tariffRs,
                          const VendorCode& vendor,
                          int itemNumber);

  static void initialize();

  const QueryGetTariffRuleRest& operator=(const QueryGetTariffRuleRest& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTariffRuleRest& operator=(const std::string& Another)
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
}; // class QueryGetTariffRuleRest

class QueryGetTariffRuleRestHistorical : public QueryGetTariffRuleRest
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTariffRuleRestHistorical(DBAdapter* dbAdapt) : QueryGetTariffRuleRest(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetTariffRuleRestHistorical() {}
  virtual const char* getQueryName() const override;

  void findTariffRuleRest(std::vector<TariffRuleRest*>& tariffRs,
                          const VendorCode& vendor,
                          int itemNumber,
                          const DateTime& startDate,
                          const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetTariffRuleRestHistorical
} // namespace tse

