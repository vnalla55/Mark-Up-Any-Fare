//----------------------------------------------------------------------------
//          File:           QueryGetGeoRule.h
//          Description:    QueryGetGeoRule
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
#include "DBAccess/GeoRuleItem.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetGeoRule : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetGeoRule(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetGeoRule(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetGeoRule() {};
  virtual const char* getQueryName() const override;

  void
  findGeoRuleItem(std::vector<GeoRuleItem*>& geoRules, const VendorCode& vendor, int itemNumber);

  static void initialize();

  const QueryGetGeoRule& operator=(const QueryGetGeoRule& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetGeoRule& operator=(const std::string& Another)
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
}; // class QueryGetGeoRule

class QueryGetGeoRuleHistorical : public QueryGetGeoRule
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetGeoRuleHistorical(DBAdapter* dbAdapt) : QueryGetGeoRule(dbAdapt, _baseSQL) {}
  virtual ~QueryGetGeoRuleHistorical() {}
  virtual const char* getQueryName() const override;

  void findGeoRuleItem(std::vector<GeoRuleItem*>& geoRules,
                       const VendorCode& vendor,
                       int itemNumber,
                       const DateTime& startDate,
                       const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetGeoRuleHistorical
} // namespace tse

