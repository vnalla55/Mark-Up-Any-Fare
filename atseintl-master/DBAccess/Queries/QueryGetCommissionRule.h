#pragma once

#include "Common/Logger.h"
#include "DBAccess/CommissionRuleInfo.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{
class QueryGetCommissionRule : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCommissionRule(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetCommissionRule(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetCommissionRule() {}
  virtual const char* getQueryName() const override;

  void findCommissionRule(std::vector<CommissionRuleInfo*>& lst,
                          const VendorCode& vendor,
                          uint64_t programId);

  static void initialize();

  const QueryGetCommissionRule& operator=(const QueryGetCommissionRule& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetCommissionRule& operator=(const std::string& Another)
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

class QueryGetCommissionRuleHistorical : public QueryGetCommissionRule
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCommissionRuleHistorical(DBAdapter* dbAdapt)
    : QueryGetCommissionRule(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetCommissionRuleHistorical() {}
  virtual const char* getQueryName() const override;

  void findCommissionRule(std::vector<CommissionRuleInfo*>& lst,
                          const VendorCode& vendor,
                          uint64_t programId,
                          DateTime startDate,
                          DateTime endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

class QueryGetAllCommissionRule : public QueryGetCommissionRule
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllCommissionRule(DBAdapter* dbAdapt)
    : QueryGetCommissionRule(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetAllCommissionRule() {}
  virtual const char* getQueryName() const override;

  void execute(std::vector<CommissionRuleInfo*>& lst) { findAllCommissionRule(lst); }
  void findAllCommissionRule(std::vector<CommissionRuleInfo*>& lst);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
} // tse
