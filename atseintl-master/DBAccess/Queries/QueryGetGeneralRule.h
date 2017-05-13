//----------------------------------------------------------------------------
//          File:           QueryGetGeneralRule.h
//          Description:    QueryGetGeneralRule
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
#include "DBAccess/GeneralRuleApp.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{

class QueryGetGeneralRule : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetGeneralRule(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetGeneralRule(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetGeneralRule() {};
  virtual const char* getQueryName() const override;

  void findGeneralRuleApp(std::vector<tse::GeneralRuleApp*>& generalRules,
                          const VendorCode& vendor,
                          const CarrierCode& carrier,
                          int ruleTariff,
                          const RuleNumber& ruleNo);

  static void initialize();

  const QueryGetGeneralRule& operator=(const QueryGetGeneralRule& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetGeneralRule& operator=(const std::string& Another)
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
}; // class QueryGetGeneralRule

class QueryGetGeneralRuleHistorical : public QueryGetGeneralRule
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetGeneralRuleHistorical(DBAdapter* dbAdapt) : QueryGetGeneralRule(dbAdapt, _baseSQL) {};
  virtual ~QueryGetGeneralRuleHistorical() {}
  virtual const char* getQueryName() const override;

  void findGeneralRuleApp(std::vector<tse::GeneralRuleApp*>& generalRules,
                          const VendorCode& vendor,
                          const CarrierCode& carrier,
                          int ruleTariff,
                          const RuleNumber& ruleNo,
                          const DateTime& startDate,
                          const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetGeneralRuleHistorical
} // namespace tse

