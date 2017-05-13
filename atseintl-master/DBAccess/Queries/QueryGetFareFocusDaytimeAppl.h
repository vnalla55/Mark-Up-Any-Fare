#pragma once

#include "DBAccess/FareFocusDaytimeApplInfo.h"
#include "DBAccess/SQLQuery.h"

#include <log4cxx/logger.h>

namespace tse
{

class QueryGetFareFocusDaytimeAppl : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareFocusDaytimeAppl(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetFareFocusDaytimeAppl(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetFareFocusDaytimeAppl() {}
  virtual const char* getQueryName() const override;

  void
  findFareFocusDaytimeAppl(std::vector<FareFocusDaytimeApplInfo*>& lst, uint64_t dayTimeApplItemNo);

  static void initialize();

  const QueryGetFareFocusDaytimeAppl& operator=(const QueryGetFareFocusDaytimeAppl& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetFareFocusDaytimeAppl& operator=(const std::string& Another)
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

class QueryGetFareFocusDaytimeApplHistorical : public QueryGetFareFocusDaytimeAppl
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareFocusDaytimeApplHistorical(DBAdapter* dbAdapt)
    : QueryGetFareFocusDaytimeAppl(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetFareFocusDaytimeApplHistorical() {}
  virtual const char* getQueryName() const override;

  void findFareFocusDaytimeAppl(std::vector<FareFocusDaytimeApplInfo*>& lst,
                              uint64_t dayTimeApplItemNo,
                              const DateTime& startDate,
                              const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

class QueryGetAllFareFocusDaytimeAppl : public QueryGetFareFocusDaytimeAppl
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllFareFocusDaytimeAppl(DBAdapter* dbAdapt)
    : QueryGetFareFocusDaytimeAppl(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetAllFareFocusDaytimeAppl() {}
  virtual const char* getQueryName() const override;

  void execute(std::vector<FareFocusDaytimeApplInfo*>& lst) { findAllFareFocusDaytimeAppl(lst); }
  void findAllFareFocusDaytimeAppl(std::vector<FareFocusDaytimeApplInfo*>& lst);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
} // tse

