#pragma once

#include "Common/Logger.h"
#include "DBAccess/FareFocusCarrierInfo.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{
class QueryGetFareFocusCarrier : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareFocusCarrier(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetFareFocusCarrier(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetFareFocusCarrier() {}
  virtual const char* getQueryName() const override;

  void
  findFareFocusCarrier(std::vector<FareFocusCarrierInfo*>& lst, uint64_t carrierItemNo);

  static void initialize();

  const QueryGetFareFocusCarrier& operator=(const QueryGetFareFocusCarrier& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetFareFocusCarrier& operator=(const std::string& Another)
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

class QueryGetFareFocusCarrierHistorical : public QueryGetFareFocusCarrier
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareFocusCarrierHistorical(DBAdapter* dbAdapt)
    : QueryGetFareFocusCarrier(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetFareFocusCarrierHistorical() {}
  virtual const char* getQueryName() const override;

  void findFareFocusCarrier(std::vector<FareFocusCarrierInfo*>& lst,
                              uint64_t carrierItemNo,
                              const DateTime& startDate,
                              const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

class QueryGetAllFareFocusCarrier : public QueryGetFareFocusCarrier
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllFareFocusCarrier(DBAdapter* dbAdapt)
    : QueryGetFareFocusCarrier(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetAllFareFocusCarrier() {}
  virtual const char* getQueryName() const override;

  void execute(std::vector<FareFocusCarrierInfo*>& lst) { findAllFareFocusCarrier(lst); }
  void findAllFareFocusCarrier(std::vector<FareFocusCarrierInfo*>& lst);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
} // tse

