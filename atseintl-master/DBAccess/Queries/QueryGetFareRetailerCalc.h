#pragma once

#include "Common/Logger.h"
#include "DBAccess/FareRetailerCalcInfo.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{
class QueryGetFareRetailerCalc : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareRetailerCalc(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetFareRetailerCalc(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetFareRetailerCalc() {}
  virtual const char* getQueryName() const override;

  void
  findFareRetailerCalc(std::vector<FareRetailerCalcInfo*>& lst, uint64_t fareRetailerCalcItemNo);

  static void initialize();

  const QueryGetFareRetailerCalc& operator=(const QueryGetFareRetailerCalc& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetFareRetailerCalc& operator=(const std::string& Another)
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

class QueryGetFareRetailerCalcHistorical : public QueryGetFareRetailerCalc
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareRetailerCalcHistorical(DBAdapter* dbAdapt)
    : QueryGetFareRetailerCalc(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetFareRetailerCalcHistorical() {}
  virtual const char* getQueryName() const override;

  void findFareRetailerCalc(std::vector<FareRetailerCalcInfo*>& lst,
                            uint64_t fareRetailerCalcItemNo,
                            const DateTime& startDate,
                            const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

class QueryGetAllFareRetailerCalc : public QueryGetFareRetailerCalc
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllFareRetailerCalc(DBAdapter* dbAdapt)
    : QueryGetFareRetailerCalc(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetAllFareRetailerCalc() {}
  virtual const char* getQueryName() const override;

  void execute(std::vector<FareRetailerCalcInfo*>& lst) { findAllFareRetailerCalc(lst); }
  void findAllFareRetailerCalc(std::vector<FareRetailerCalcInfo*>& lst);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
} // tse



