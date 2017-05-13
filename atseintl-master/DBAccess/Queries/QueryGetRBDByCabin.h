#pragma once

#include "Common/Logger.h"
#include "DBAccess/RBDByCabinInfo.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{
class QueryGetRBDByCabin : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetRBDByCabin(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetRBDByCabin(DBAdapter* dbAdapt,
                     const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetRBDByCabin() {}
  virtual const char* getQueryName() const override;

  void findRBDByCabin(std::vector<RBDByCabinInfo*>& lst,
                      const VendorCode& vendor,
                      const CarrierCode& carrier);

  static void initialize();

  const QueryGetRBDByCabin& operator=(const QueryGetRBDByCabin& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetRBDByCabin& operator=(const std::string& Another)
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

class QueryGetRBDByCabinHistorical : public QueryGetRBDByCabin
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetRBDByCabinHistorical(DBAdapter* dbAdapt)
    : QueryGetRBDByCabin(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetRBDByCabinHistorical() {}
  virtual const char* getQueryName() const override;

  void findRBDByCabin(std::vector<RBDByCabinInfo*>& lst,
                      const VendorCode& vendor,
                      const CarrierCode& carrier,
                      const DateTime& startDate,
                      const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

class QueryGetAllRBDByCabin : public QueryGetRBDByCabin
{
private:
  template <typename Query> friend class DBAccessTestHelper;

  template <typename Query> friend class SQLQueryInitializerHelper;

  static void deinitialize() { _isInitialized = false; }
public:

  QueryGetAllRBDByCabin(DBAdapter* dbAdapt)
    : QueryGetRBDByCabin(dbAdapt, _baseSQL)
  {
  }

  virtual ~QueryGetAllRBDByCabin() {}
  virtual const char* getQueryName() const override;

  void execute(std::vector<RBDByCabinInfo*>& lst) { findAllRBDByCabin(lst); }
  void findAllRBDByCabin(std::vector<RBDByCabinInfo*>& lst);
  static void initialize();
private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

} // tse

