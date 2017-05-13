#pragma once

#include "Common/Logger.h"
#include "DBAccess/FareFocusRoutingInfo.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{
class QueryGetFareFocusRouting : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareFocusRouting(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetFareFocusRouting(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetFareFocusRouting() {}
  virtual const char* getQueryName() const override;

  void
  findFareFocusRouting(std::vector<FareFocusRoutingInfo*>& lst, uint64_t routingItemNo);

  static void initialize();

  const QueryGetFareFocusRouting& operator=(const QueryGetFareFocusRouting& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetFareFocusRouting& operator=(const std::string& Another)
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

class QueryGetFareFocusRoutingHistorical : public QueryGetFareFocusRouting
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareFocusRoutingHistorical(DBAdapter* dbAdapt)
    : QueryGetFareFocusRouting(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetFareFocusRoutingHistorical() {}
  virtual const char* getQueryName() const override;

  void findFareFocusRouting(std::vector<FareFocusRoutingInfo*>& lst,
                              uint64_t routingItemNo,
                              const DateTime& startDate,
                              const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

class QueryGetAllFareFocusRouting : public QueryGetFareFocusRouting
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllFareFocusRouting(DBAdapter* dbAdapt)
    : QueryGetFareFocusRouting(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetAllFareFocusRouting() {}
  virtual const char* getQueryName() const override;

  void execute(std::vector<FareFocusRoutingInfo*>& lst) { findAllFareFocusRouting(lst); }
  void findAllFareFocusRouting(std::vector<FareFocusRoutingInfo*>& lst);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
} // tse

