//----------------------------------------------------------------------------
//          File:           QueryGetRuleAppl.h
//          Description:    QueryGetRuleAppl
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
#include "DBAccess/RuleApplication.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{

class QueryGetRuleAppl : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetRuleAppl(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetRuleAppl(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetRuleAppl() {};
  virtual const char* getQueryName() const override;

  void findRuleApplication(std::vector<tse::RuleApplication*>& ruleAppl,
                           const VendorCode& vendor,
                           int itemNo);

  static void initialize();

  const QueryGetRuleAppl& operator=(const QueryGetRuleAppl& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetRuleAppl& operator=(const std::string& Another)
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
}; // class QueryGetRuleAppl

class QueryGetRuleApplHistorical : public QueryGetRuleAppl
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetRuleApplHistorical(DBAdapter* dbAdapt) : QueryGetRuleAppl(dbAdapt, _baseSQL) {}
  virtual ~QueryGetRuleApplHistorical() {}
  virtual const char* getQueryName() const override;

  void findRuleApplication(std::vector<tse::RuleApplication*>& ruleAppl,
                           const VendorCode& vendor,
                           int itemNo,
                           const DateTime& startDate,
                           const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetRuleApplHistorical
} // namespace tse

