#pragma once

#include "Common/Logger.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{
class FareFocusLookupInfo;

class QueryGetFareFocusLookup : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareFocusLookup(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetFareFocusLookup(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetFareFocusLookup() {}
  virtual const char* getQueryName() const override;

  void
  findFareFocusLookup(std::vector<FareFocusLookupInfo*>& lst,
                      const PseudoCityCode& pcc);

  static void initialize();

  const QueryGetFareFocusLookup& operator=(const QueryGetFareFocusLookup& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetFareFocusLookup& operator=(const std::string& Another)
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

class QueryGetFareFocusLookupHistorical : public QueryGetFareFocusLookup
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareFocusLookupHistorical(DBAdapter* dbAdapt)
    : QueryGetFareFocusLookup(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetFareFocusLookupHistorical() {}
  virtual const char* getQueryName() const override;

  void findFareFocusLookup(std::vector<FareFocusLookupInfo*>& lst,
                           const PseudoCityCode& pcc,
                           const DateTime& startDate,
                           const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

} // tse

