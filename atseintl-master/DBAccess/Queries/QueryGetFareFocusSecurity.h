#pragma once

#include "Common/Logger.h"
#include "DBAccess/FareFocusSecurityInfo.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{
class QueryGetFareFocusSecurity : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareFocusSecurity(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetFareFocusSecurity(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetFareFocusSecurity() {}
  virtual const char* getQueryName() const override;

  void
  findFareFocusSecurity(std::vector<FareFocusSecurityInfo*>& lst, uint64_t securityItemNo);

  static void initialize();

  const QueryGetFareFocusSecurity& operator=(const QueryGetFareFocusSecurity& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetFareFocusSecurity& operator=(const std::string& Another)
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

class QueryGetFareFocusSecurityHistorical : public QueryGetFareFocusSecurity
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareFocusSecurityHistorical(DBAdapter* dbAdapt)
    : QueryGetFareFocusSecurity(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetFareFocusSecurityHistorical() {}
  virtual const char* getQueryName() const override;

  void findFareFocusSecurity(std::vector<FareFocusSecurityInfo*>& lst,
                              uint64_t securityItemNo,
                              const DateTime& startDate,
                              const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

class QueryGetAllFareFocusSecurity : public QueryGetFareFocusSecurity
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllFareFocusSecurity(DBAdapter* dbAdapt)
    : QueryGetFareFocusSecurity(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetAllFareFocusSecurity() {}
  virtual const char* getQueryName() const override;

  void execute(std::vector<FareFocusSecurityInfo*>& lst) { findAllFareFocusSecurity(lst); }
  void findAllFareFocusSecurity(std::vector<FareFocusSecurityInfo*>& lst);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
} // tse

