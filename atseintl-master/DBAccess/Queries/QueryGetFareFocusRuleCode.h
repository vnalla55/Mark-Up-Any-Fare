#pragma once

#include "Common/Logger.h"
#include "DBAccess/FareFocusRuleCodeInfo.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{
class QueryGetFareFocusRuleCode : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareFocusRuleCode(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetFareFocusRuleCode(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetFareFocusRuleCode() {}
  virtual const char* getQueryName() const override;

  void
  findFareFocusRuleCode(std::vector<FareFocusRuleCodeInfo*>& lst, uint64_t ruleCdItemNo);

  static void initialize();

  const QueryGetFareFocusRuleCode& operator=(const QueryGetFareFocusRuleCode& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetFareFocusRuleCode& operator=(const std::string& Another)
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

class QueryGetFareFocusRuleCodeHistorical : public QueryGetFareFocusRuleCode
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareFocusRuleCodeHistorical(DBAdapter* dbAdapt)
    : QueryGetFareFocusRuleCode(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetFareFocusRuleCodeHistorical() {}
  virtual const char* getQueryName() const override;

  void findFareFocusRuleCode(std::vector<FareFocusRuleCodeInfo*>& lst,
                             uint64_t ruleCdItemNo,
                             const DateTime& startDate,
                             const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

class QueryGetAllFareFocusRuleCode : public QueryGetFareFocusRuleCode
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllFareFocusRuleCode(DBAdapter* dbAdapt)
    : QueryGetFareFocusRuleCode(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetAllFareFocusRuleCode() {}
  virtual const char* getQueryName() const override;

  void execute(std::vector<FareFocusRuleCodeInfo*>& lst) { findAllFareFocusRuleCode(lst); }
  void findAllFareFocusRuleCode(std::vector<FareFocusRuleCodeInfo*>& lst);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
} // tse


