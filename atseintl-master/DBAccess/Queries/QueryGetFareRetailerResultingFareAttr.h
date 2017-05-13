#pragma once

#include "Common/Logger.h"
#include "DBAccess/FareRetailerResultingFareAttrInfo.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{
class QueryGetFareRetailerResultingFareAttr : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareRetailerResultingFareAttr(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetFareRetailerResultingFareAttr(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetFareRetailerResultingFareAttr() {}
  virtual const char* getQueryName() const override;

  void
  findFareRetailerResultingFareAttr (std::vector<FareRetailerResultingFareAttrInfo*>& lst, uint64_t resultingFareAttrItemNo);

  static void initialize();

  const QueryGetFareRetailerResultingFareAttr& operator=(const QueryGetFareRetailerResultingFareAttr& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetFareRetailerResultingFareAttr& operator=(const std::string& Another)
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

class QueryGetFareRetailerResultingFareAttrHistorical : public QueryGetFareRetailerResultingFareAttr
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareRetailerResultingFareAttrHistorical(DBAdapter* dbAdapt)
    : QueryGetFareRetailerResultingFareAttr(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetFareRetailerResultingFareAttrHistorical() {}
  virtual const char* getQueryName() const override;

  void findFareRetailerResultingFareAttr(std::vector<FareRetailerResultingFareAttrInfo*>& lst,
                                         uint64_t resultingFareAttrItemNo,
                                         const DateTime& startDate,
                                         const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

class QueryGetAllFareRetailerResultingFareAttr : public QueryGetFareRetailerResultingFareAttr
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllFareRetailerResultingFareAttr(DBAdapter* dbAdapt)
    : QueryGetFareRetailerResultingFareAttr(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetAllFareRetailerResultingFareAttr() {}
  virtual const char* getQueryName() const override;

  void execute(std::vector<FareRetailerResultingFareAttrInfo*>& lst) { findAllFareRetailerResultingFareAttr(lst); }
  void findAllFareRetailerResultingFareAttr(std::vector<FareRetailerResultingFareAttrInfo*>& lst);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

} // tse




