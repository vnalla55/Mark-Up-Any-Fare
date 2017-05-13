#pragma once

#include "Common/Logger.h"
#include "DBAccess/FareFocusFareClassInfo.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{
class QueryGetFareFocusFareClass : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareFocusFareClass(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetFareFocusFareClass(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetFareFocusFareClass() {}
  virtual const char* getQueryName() const override;

  void
  findFareFocusFareClass(std::vector<FareFocusFareClassInfo*>& lst, uint64_t fareClassItemNo);

  static void initialize();

  const QueryGetFareFocusFareClass& operator=(const QueryGetFareFocusFareClass& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetFareFocusFareClass& operator=(const std::string& Another)
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

class QueryGetFareFocusFareClassHistorical : public QueryGetFareFocusFareClass
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareFocusFareClassHistorical(DBAdapter* dbAdapt)
    : QueryGetFareFocusFareClass(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetFareFocusFareClassHistorical() {}
  virtual const char* getQueryName() const override;

  void findFareFocusFareClass(std::vector<FareFocusFareClassInfo*>& lst,
                              uint64_t fareClassItemNo,
                              const DateTime& startDate,
                              const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

class QueryGetAllFareFocusFareClass : public QueryGetFareFocusFareClass
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllFareFocusFareClass(DBAdapter* dbAdapt)
    : QueryGetFareFocusFareClass(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetAllFareFocusFareClass() {}
  virtual const char* getQueryName() const override;

  void execute(std::vector<FareFocusFareClassInfo*>& lst) { findAllFareFocusFareClass(lst); }
  void findAllFareFocusFareClass(std::vector<FareFocusFareClassInfo*>& lst);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
} // tse

