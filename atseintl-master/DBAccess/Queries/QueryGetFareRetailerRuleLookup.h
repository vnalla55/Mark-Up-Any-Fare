#pragma once

#include "Common/Logger.h"
#include "DBAccess/FareRetailerRuleLookupInfo.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{
class FareRetailerRuleLookupInfo;

class QueryGetFareRetailerRuleLookup : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareRetailerRuleLookup(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetFareRetailerRuleLookup(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetFareRetailerRuleLookup() {}
  virtual const char* getQueryName() const override;

  void
  findFareRetailerRuleLookup(std::vector<FareRetailerRuleLookupInfo*>& lst,
                             Indicator applicationType,
                             const PseudoCityCode& sourcePcc,
                             const PseudoCityCode& pcc);

  static void initialize();

  const QueryGetFareRetailerRuleLookup& operator=(const QueryGetFareRetailerRuleLookup& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetFareRetailerRuleLookup& operator=(const std::string& Another)
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

class QueryGetFareRetailerRuleLookupHistorical : public QueryGetFareRetailerRuleLookup
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareRetailerRuleLookupHistorical(DBAdapter* dbAdapt)
    : QueryGetFareRetailerRuleLookup(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetFareRetailerRuleLookupHistorical() {}
  virtual const char* getQueryName() const override;

  void findFareRetailerRuleLookup(std::vector<FareRetailerRuleLookupInfo*>& lst,
                                  Indicator applicationType,
                                  const PseudoCityCode& sourcePcc,
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


