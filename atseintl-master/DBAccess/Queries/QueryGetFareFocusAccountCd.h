#pragma once

#include "Common/Logger.h"
#include "DBAccess/FareFocusAccountCdInfo.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{
class QueryGetFareFocusAccountCd : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareFocusAccountCd(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetFareFocusAccountCd(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetFareFocusAccountCd() {}
  virtual const char* getQueryName() const override;

  void
  findFareFocusAccountCd(std::vector<FareFocusAccountCdInfo*>& lst, uint64_t accountCdItemNo);

  static void initialize();

  const QueryGetFareFocusAccountCd& operator=(const QueryGetFareFocusAccountCd& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetFareFocusAccountCd& operator=(const std::string& Another)
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

class QueryGetFareFocusAccountCdHistorical : public QueryGetFareFocusAccountCd
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareFocusAccountCdHistorical(DBAdapter* dbAdapt)
    : QueryGetFareFocusAccountCd(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetFareFocusAccountCdHistorical() {}
  virtual const char* getQueryName() const override;

  void findFareFocusAccountCd(std::vector<FareFocusAccountCdInfo*>& lst,
                              uint64_t accountCdItemNo,
                              const DateTime& startDate,
                              const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

class QueryGetAllFareFocusAccountCd : public QueryGetFareFocusAccountCd
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllFareFocusAccountCd(DBAdapter* dbAdapt)
    : QueryGetFareFocusAccountCd(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetAllFareFocusAccountCd() {}
  virtual const char* getQueryName() const override;

  void execute(std::vector<FareFocusAccountCdInfo*>& lst) { findAllFareFocusAccountCd(lst); }
  void findAllFareFocusAccountCd(std::vector<FareFocusAccountCdInfo*>& lst);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
} // tse

