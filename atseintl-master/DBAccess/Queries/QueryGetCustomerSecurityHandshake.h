#pragma once

#include "Common/Logger.h"
#include "DBAccess/CustomerSecurityHandshakeInfo.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{
class QueryGetCustomerSecurityHandshake : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCustomerSecurityHandshake(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetCustomerSecurityHandshake(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetCustomerSecurityHandshake() {}
  virtual const char* getQueryName() const override;

  void
  findCustomerSecurityHandshake(std::vector<CustomerSecurityHandshakeInfo*>& lst,
                                const PseudoCityCode& pcc,
                                const Code<8>& productCD);

  static void initialize();

  const QueryGetCustomerSecurityHandshake& operator=(const QueryGetCustomerSecurityHandshake& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetCustomerSecurityHandshake& operator=(const std::string& Another)
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

class QueryGetCustomerSecurityHandshakeHistorical : public QueryGetCustomerSecurityHandshake
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCustomerSecurityHandshakeHistorical(DBAdapter* dbAdapt)
    : QueryGetCustomerSecurityHandshake(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetCustomerSecurityHandshakeHistorical() {}
  virtual const char* getQueryName() const override;

  void findCustomerSecurityHandshake(std::vector<CustomerSecurityHandshakeInfo*>& lst,
                                     const PseudoCityCode& pcc,
                                     const Code<8>& productCode,
                                     const DateTime& startDate,
                                     const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

class QueryGetAllCustomerSecurityHandshake : public QueryGetCustomerSecurityHandshake
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllCustomerSecurityHandshake(DBAdapter* dbAdapt)
    : QueryGetCustomerSecurityHandshake(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetAllCustomerSecurityHandshake() {}
  virtual const char* getQueryName() const override;

  void execute(std::vector<CustomerSecurityHandshakeInfo*>& lst) { findAllCustomerSecurityHandshake(lst); }
  void findAllCustomerSecurityHandshake(std::vector<CustomerSecurityHandshakeInfo*>& lst);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
} // tse

