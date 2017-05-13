//----------------------------------------------------------------------------
//          File:           QueryGetDateOverrideRule.h
//          Description:    QueryGetDateOverrideRule
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
#include "DBAccess/DateOverrideRuleItem.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetDateOverrideRule : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetDateOverrideRule(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetDateOverrideRule(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetDateOverrideRule() {};

  virtual const char* getQueryName() const override;

  void findDateOverrideRuleItem(std::vector<DateOverrideRuleItem*>& dateOverrides,
                                const VendorCode& vendor,
                                int itemNumber);

  static void initialize();

  const QueryGetDateOverrideRule& operator=(const QueryGetDateOverrideRule& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetDateOverrideRule& operator=(const std::string& Another)
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
}; // class QueryGetDateOverrideRule

class QueryGetDateOverrideRuleHistorical : public QueryGetDateOverrideRule
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetDateOverrideRuleHistorical(DBAdapter* dbAdapt)
    : QueryGetDateOverrideRule(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetDateOverrideRuleHistorical() {}

  virtual const char* getQueryName() const override;

  void findDateOverrideRuleItem(std::vector<DateOverrideRuleItem*>& dateOverrides,
                                const VendorCode& vendor,
                                int itemNumber,
                                const DateTime& startDate,
                                const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetDateOverrideRuleHistorical
} // namespace tse

