#pragma once

#include "Common/Logger.h"
#include "DBAccess/FareFocusPsgTypeInfo.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{
class QueryGetFareFocusPsgType : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareFocusPsgType(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetFareFocusPsgType(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetFareFocusPsgType() {}
  virtual const char* getQueryName() const override;

  void
  findFareFocusPsgType(std::vector<FareFocusPsgTypeInfo*>& lst, uint64_t psgTypeItemNo);

  static void initialize();

  const QueryGetFareFocusPsgType& operator=(const QueryGetFareFocusPsgType& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetFareFocusPsgType& operator=(const std::string& Another)
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

class QueryGetFareFocusPsgTypeHistorical : public QueryGetFareFocusPsgType
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareFocusPsgTypeHistorical(DBAdapter* dbAdapt)
    : QueryGetFareFocusPsgType(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetFareFocusPsgTypeHistorical() {}
  virtual const char* getQueryName() const override;

  void findFareFocusPsgType(std::vector<FareFocusPsgTypeInfo*>& lst,
                              uint64_t psgTypeItemNo,
                              const DateTime& startDate,
                              const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

class QueryGetAllFareFocusPsgType : public QueryGetFareFocusPsgType
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllFareFocusPsgType(DBAdapter* dbAdapt)
    : QueryGetFareFocusPsgType(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetAllFareFocusPsgType() {}
  virtual const char* getQueryName() const override;

  void execute(std::vector<FareFocusPsgTypeInfo*>& lst) { findAllFareFocusPsgType(lst); }
  void findAllFareFocusPsgType(std::vector<FareFocusPsgTypeInfo*>& lst);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
} // tse

