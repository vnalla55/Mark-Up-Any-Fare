//----------------------------------------------------------------------------
//          File:           QueryGetRoundTripRule.h
//          Description:    QueryGetRoundTripRule
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
#include "DBAccess/RoundTripRuleItem.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{

class QueryGetRoundTripRule : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetRoundTripRule(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetRoundTripRule(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetRoundTripRule() {}
  virtual const char* getQueryName() const override;

  void findRoundTripRuleItem(std::vector<RoundTripRuleItem*>& ctRules,
                             const VendorCode& vendor,
                             int itemNumber);

  static void initialize();

  const QueryGetRoundTripRule& operator=(const QueryGetRoundTripRule& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetRoundTripRule& operator=(const std::string& Another)
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
}; // class QueryGetRoundTripRule

class QueryGetRoundTripRuleHistorical : public QueryGetRoundTripRule
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetRoundTripRuleHistorical(DBAdapter* dbAdapt) : QueryGetRoundTripRule(dbAdapt, _baseSQL) {}
  virtual ~QueryGetRoundTripRuleHistorical() {}
  virtual const char* getQueryName() const override;

  void findRoundTripRuleItem(std::vector<RoundTripRuleItem*>& ctRules,
                             const VendorCode& vendor,
                             int itemNumber,
                             const DateTime& startDate,
                             const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetRoundTripRuleHistorical
} // namespace tse

