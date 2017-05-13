#pragma once

#include "Common/Logger.h"
#include "DBAccess/FareFocusRuleInfo.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{
class QueryGetFareFocusRule : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareFocusRule(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetFareFocusRule(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetFareFocusRule() {}
  virtual const char* getQueryName() const override;

  void
  findFareFocusRule(std::vector<FareFocusRuleInfo*>& lst, uint64_t fareFocusRuleId);

  static void initialize();

  const QueryGetFareFocusRule& operator=(const QueryGetFareFocusRule& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetFareFocusRule& operator=(const std::string& Another)
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

class QueryGetFareFocusRuleHistorical : public QueryGetFareFocusRule
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareFocusRuleHistorical(DBAdapter* dbAdapt)
    : QueryGetFareFocusRule(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetFareFocusRuleHistorical() {}
  virtual const char* getQueryName() const override;

  void findFareFocusRule(std::vector<FareFocusRuleInfo*>& lst,
                         uint64_t fareFocusRuleId,
                         const DateTime& startDate,
                         const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

class QueryGetAllFareFocusRule : public QueryGetFareFocusRule
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllFareFocusRule(DBAdapter* dbAdapt)
    : QueryGetFareFocusRule(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetAllFareFocusRule() {}
  virtual const char* getQueryName() const override;

  void execute(std::vector<FareFocusRuleInfo*>& lst) { findAllFareFocusRule(lst); }
  void findAllFareFocusRule(std::vector<FareFocusRuleInfo*>& lst);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
} // tse

