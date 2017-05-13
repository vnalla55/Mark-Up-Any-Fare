#pragma once

#include "Common/Logger.h"
#include "DBAccess/CommissionContractInfo.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{
class QueryGetCommissionContract : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCommissionContract(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetCommissionContract(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetCommissionContract() {}
  virtual const char* getQueryName() const override;

  void findCommissionContract(std::vector<CommissionContractInfo*>& lst,
                              const VendorCode& vendor,
                              const CarrierCode& carrier,
                              const PseudoCityCode& pcc);

  static void initialize();

  const QueryGetCommissionContract& operator=(const QueryGetCommissionContract& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetCommissionContract& operator=(const std::string& Another)
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

class QueryGetCommissionContractHistorical : public QueryGetCommissionContract
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCommissionContractHistorical(DBAdapter* dbAdapt)
    : QueryGetCommissionContract(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetCommissionContractHistorical() {}
  virtual const char* getQueryName() const override;

  void findCommissionContract(std::vector<CommissionContractInfo*>& lst,
                              const VendorCode& vendor,
                              const CarrierCode& carrier,
                              const PseudoCityCode& pcc,
                              DateTime startDate,
                              DateTime endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

class QueryGetAllCommissionContract : public QueryGetCommissionContract
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllCommissionContract(DBAdapter* dbAdapt)
    : QueryGetCommissionContract(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetAllCommissionContract() {}
  virtual const char* getQueryName() const override;

  void execute(std::vector<CommissionContractInfo*>& lst) { findAllCommissionContract(lst); }
  void findAllCommissionContract(std::vector<CommissionContractInfo*>& lst);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
} // tse
