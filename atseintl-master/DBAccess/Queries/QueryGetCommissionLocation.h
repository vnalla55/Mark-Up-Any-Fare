#pragma once

#include "Common/Logger.h"
#include "DBAccess/CommissionLocSegInfo.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{
class QueryGetCommissionLocation : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCommissionLocation(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetCommissionLocation(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetCommissionLocation() {}
  virtual const char* getQueryName() const override;

  void findCommissionLocation(std::vector<CommissionLocSegInfo*>& lst,
                              const VendorCode& vendor,
                              uint64_t contractId);

  static void initialize();

  const QueryGetCommissionLocation& operator=(const QueryGetCommissionLocation& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetCommissionLocation& operator=(const std::string& Another)
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

class QueryGetCommissionLocationHistorical : public QueryGetCommissionLocation
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCommissionLocationHistorical(DBAdapter* dbAdapt)
    : QueryGetCommissionLocation(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetCommissionLocationHistorical() {}
  virtual const char* getQueryName() const override;

  void findCommissionLocation(std::vector<CommissionLocSegInfo*>& lst,
                              const VendorCode& vendor,
                              uint64_t contractId,
                              DateTime startDate,
                              DateTime endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

} // tse
