#pragma once

#include "Common/Logger.h"
#include "DBAccess/FareRetailerRuleInfo.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{
class QueryGetFareRetailerRule : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareRetailerRule(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetFareRetailerRule(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetFareRetailerRule() {}
  virtual const char* getQueryName() const override;

  void
  findFareRetailerRule(std::vector<FareRetailerRuleInfo*>& lst, uint64_t fareRetailerRuleId);

  static void initialize();

  const QueryGetFareRetailerRule& operator=(const QueryGetFareRetailerRule& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetFareRetailerRule& operator=(const std::string& Another)
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

class QueryGetFareRetailerRuleHistorical : public QueryGetFareRetailerRule
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareRetailerRuleHistorical(DBAdapter* dbAdapt)
    : QueryGetFareRetailerRule(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetFareRetailerRuleHistorical() {}
  virtual const char* getQueryName() const override;

  void findFareRetailerRule(std::vector<FareRetailerRuleInfo*>& lst,
                            uint64_t fareRetailerRuleId,
                            const DateTime& startDate,
                            const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

class QueryGetAllFareRetailerRule : public QueryGetFareRetailerRule
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllFareRetailerRule(DBAdapter* dbAdapt)
    : QueryGetFareRetailerRule(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetAllFareRetailerRule() {}
  virtual const char* getQueryName() const override;

  void execute(std::vector<FareRetailerRuleInfo*>& lst) { findAllFareRetailerRule(lst); }
  void findAllFareRetailerRule(std::vector<FareRetailerRuleInfo*>& lst);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

} // tse



