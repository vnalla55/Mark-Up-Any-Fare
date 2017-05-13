#pragma once

#include "Common/Logger.h"
#include "DBAccess/CommissionMarketSegInfo.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{
class QueryGetCommissionMarket : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCommissionMarket(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetCommissionMarket(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetCommissionMarket() {}
  virtual const char* getQueryName() const override;

  void findCommissionMarket(std::vector<CommissionMarketSegInfo*>& lst,
                            const VendorCode& vendor,
                            uint64_t contractId);

  static void initialize();

  const QueryGetCommissionMarket& operator=(const QueryGetCommissionMarket& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetCommissionMarket& operator=(const std::string& Another)
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

class QueryGetCommissionMarketHistorical : public QueryGetCommissionMarket
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCommissionMarketHistorical(DBAdapter* dbAdapt)
    : QueryGetCommissionMarket(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetCommissionMarketHistorical() {}
  virtual const char* getQueryName() const override;

  void findCommissionMarket(std::vector<CommissionMarketSegInfo*>& lst,
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
