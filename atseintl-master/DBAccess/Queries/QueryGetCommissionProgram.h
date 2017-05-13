#pragma once

#include "Common/Logger.h"
#include "DBAccess/CommissionProgramInfo.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{
class QueryGetCommissionProgram : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCommissionProgram(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetCommissionProgram(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetCommissionProgram() {}
  virtual const char* getQueryName() const override;

  void findCommissionProgram(std::vector<CommissionProgramInfo*>& lst,
                             const VendorCode& vendor,
                             uint64_t contractId);

  static void initialize();

  const QueryGetCommissionProgram& operator=(const QueryGetCommissionProgram& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetCommissionProgram& operator=(const std::string& Another)
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

class QueryGetCommissionProgramHistorical : public QueryGetCommissionProgram
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCommissionProgramHistorical(DBAdapter* dbAdapt)
    : QueryGetCommissionProgram(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetCommissionProgramHistorical() {}
  virtual const char* getQueryName() const override;

  void findCommissionProgram(std::vector<CommissionProgramInfo*>& lst,
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

class QueryGetAllCommissionProgram : public QueryGetCommissionProgram
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllCommissionProgram(DBAdapter* dbAdapt)
    : QueryGetCommissionProgram(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetAllCommissionProgram() {}
  virtual const char* getQueryName() const override;

  void execute(std::vector<CommissionProgramInfo*>& lst) { findAllCommissionProgram(lst); }
  void findAllCommissionProgram(std::vector<CommissionProgramInfo*>& lst);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
} // tse
