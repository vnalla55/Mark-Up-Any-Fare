#pragma once

#include "Common/Logger.h"
#include "DBAccess/FareFocusDisplayCatTypeInfo.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{
class QueryGetFareFocusDisplayCatType : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareFocusDisplayCatType(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetFareFocusDisplayCatType(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetFareFocusDisplayCatType() {}
  virtual const char* getQueryName() const override;

  void
  findFareFocusDisplayCatType(std::vector<FareFocusDisplayCatTypeInfo*>& lst, uint64_t displayCatTypeItemNo);

  static void initialize();

  const QueryGetFareFocusDisplayCatType& operator=(const QueryGetFareFocusDisplayCatType& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetFareFocusDisplayCatType& operator=(const std::string& Another)
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

class QueryGetFareFocusDisplayCatTypeHistorical : public QueryGetFareFocusDisplayCatType
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareFocusDisplayCatTypeHistorical(DBAdapter* dbAdapt)
    : QueryGetFareFocusDisplayCatType(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetFareFocusDisplayCatTypeHistorical() {}
  virtual const char* getQueryName() const override;

  void findFareFocusDisplayCatType(std::vector<FareFocusDisplayCatTypeInfo*>& lst,
                              uint64_t displayCatTypeItemNo,
                              const DateTime& startDate,
                              const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

class QueryGetAllFareFocusDisplayCatType : public QueryGetFareFocusDisplayCatType
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllFareFocusDisplayCatType(DBAdapter* dbAdapt)
    : QueryGetFareFocusDisplayCatType(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetAllFareFocusDisplayCatType() {}
  virtual const char* getQueryName() const override;

  void execute(std::vector<FareFocusDisplayCatTypeInfo*>& lst) { findAllFareFocusDisplayCatType(lst); }
  void findAllFareFocusDisplayCatType(std::vector<FareFocusDisplayCatTypeInfo*>& lst);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
} // tse

