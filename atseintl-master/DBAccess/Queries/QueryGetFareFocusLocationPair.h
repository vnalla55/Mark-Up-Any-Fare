#pragma once

#include "Common/Logger.h"
#include "DBAccess/FareFocusLocationPairInfo.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{
class QueryGetFareFocusLocationPair : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareFocusLocationPair(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetFareFocusLocationPair(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetFareFocusLocationPair() {}
  virtual const char* getQueryName() const override;

  void
  findFareFocusLocationPair(std::vector<FareFocusLocationPairInfo*>& lst, uint64_t locationPairItemNo);

  static void initialize();

  const QueryGetFareFocusLocationPair& operator=(const QueryGetFareFocusLocationPair& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetFareFocusLocationPair& operator=(const std::string& Another)
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

class QueryGetFareFocusLocationPairHistorical : public QueryGetFareFocusLocationPair
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareFocusLocationPairHistorical(DBAdapter* dbAdapt)
    : QueryGetFareFocusLocationPair(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetFareFocusLocationPairHistorical() {}
  virtual const char* getQueryName() const override;

  void findFareFocusLocationPair(std::vector<FareFocusLocationPairInfo*>& lst,
                              uint64_t locationPairItemNo,
                              const DateTime& startDate,
                              const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

class QueryGetAllFareFocusLocationPair : public QueryGetFareFocusLocationPair
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllFareFocusLocationPair(DBAdapter* dbAdapt)
    : QueryGetFareFocusLocationPair(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetAllFareFocusLocationPair() {}
  virtual const char* getQueryName() const override;

  void execute(std::vector<FareFocusLocationPairInfo*>& lst) { findAllFareFocusLocationPair(lst); }
  void findAllFareFocusLocationPair(std::vector<FareFocusLocationPairInfo*>& lst);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
} // tse

