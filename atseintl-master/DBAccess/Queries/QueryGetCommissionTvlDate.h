#pragma once

#include "Common/Logger.h"
#include "DBAccess/CommissionTravelDatesSegInfo.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{
class QueryGetCommissionTvlDate : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCommissionTvlDate(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetCommissionTvlDate(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetCommissionTvlDate() {}
  virtual const char* getQueryName() const override;

  void findCommissionTvlDate(std::vector<CommissionTravelDatesSegInfo*>& lst,
                             const VendorCode& vendor,
                             uint64_t contractId);

  static void initialize();

  const QueryGetCommissionTvlDate& operator=(const QueryGetCommissionTvlDate& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetCommissionTvlDate& operator=(const std::string& Another)
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

class QueryGetCommissionTvlDateHistorical : public QueryGetCommissionTvlDate
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCommissionTvlDateHistorical(DBAdapter* dbAdapt)
    : QueryGetCommissionTvlDate(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetCommissionTvlDateHistorical() {}
  virtual const char* getQueryName() const override;

  void findCommissionTvlDate(std::vector<CommissionTravelDatesSegInfo*>& lst,
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
