#pragma once

#include "Common/Logger.h"
#include "DBAccess/FareFocusBookingCodeInfo.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{
class QueryGetFareFocusBookingCode : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareFocusBookingCode(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetFareFocusBookingCode(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetFareFocusBookingCode() {}
  virtual const char* getQueryName() const override;

  void
  findFareFocusBookingCode(std::vector<FareFocusBookingCodeInfo*>& lst, uint64_t bookingCodeItemNo);

  static void initialize();

  const QueryGetFareFocusBookingCode& operator=(const QueryGetFareFocusBookingCode& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetFareFocusBookingCode& operator=(const std::string& Another)
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

class QueryGetFareFocusBookingCodeHistorical : public QueryGetFareFocusBookingCode
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareFocusBookingCodeHistorical(DBAdapter* dbAdapt)
    : QueryGetFareFocusBookingCode(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetFareFocusBookingCodeHistorical() {}
  virtual const char* getQueryName() const override;

  void findFareFocusBookingCode(std::vector<FareFocusBookingCodeInfo*>& lst,
                                uint64_t bookingCodeItemNo,
                                const DateTime& startDate,
                                const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

class QueryGetAllFareFocusBookingCode : public QueryGetFareFocusBookingCode
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllFareFocusBookingCode(DBAdapter* dbAdapt)
    : QueryGetFareFocusBookingCode(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetAllFareFocusBookingCode() {}
  virtual const char* getQueryName() const override;

  void execute(std::vector<FareFocusBookingCodeInfo*>& lst) { findAllFareFocusBookingCode(lst); }
  void findAllFareFocusBookingCode(std::vector<FareFocusBookingCodeInfo*>& lst);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
} // tse

