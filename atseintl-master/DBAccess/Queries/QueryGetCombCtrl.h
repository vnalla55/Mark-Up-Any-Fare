//----------------------------------------------------------------------------
//          File:           QueryGetCombCtrl.h
//          Description:    QueryGetCombCtrl
//          Created:        3/2/2006
//          Authors:        Mike Lillis
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
#include "DBAccess/CombinabilityRuleInfo.h"
#include "DBAccess/CombinabilityRuleItemInfo.h"
#include "DBAccess/ScoreSummary.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{

class QueryGetCombCtrl : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCombCtrl(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetCombCtrl(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetCombCtrl() {};
  virtual const char* getQueryName() const override;

  void findCombinabilityRule(std::vector<tse::CombinabilityRuleInfo*>& combs,
                             const VendorCode& vendor,
                             const CarrierCode& carrier,
                             int ruleTariff,
                             const RuleNumber& ruleNo,
                             int cat);

  static void initialize();

  const QueryGetCombCtrl& operator=(const QueryGetCombCtrl& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetCombCtrl& operator=(const std::string& Another)
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
}; // class QueryGetCombCtrl

class QueryGetCombCtrlHistorical : public QueryGetCombCtrl
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCombCtrlHistorical(DBAdapter* dbAdapt) : QueryGetCombCtrl(dbAdapt, _baseSQL) {};
  virtual ~QueryGetCombCtrlHistorical() {};
  virtual const char* getQueryName() const override;

  void findCombinabilityRule(std::vector<tse::CombinabilityRuleInfo*>& combs,
                             const VendorCode& vendor,
                             const CarrierCode& carrier,
                             int ruleTariff,
                             const RuleNumber& ruleNo,
                             int cat,
                             const DateTime& startDate,
                             const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetCombCtrlHistorical
} // namespace tse
