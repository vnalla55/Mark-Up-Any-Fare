#pragma once

#include "DBAccess/SQLQuery.h"

namespace tse
{
class ServicesDescription;

class QueryGetServicesDescription : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetServicesDescription(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}

  QueryGetServicesDescription(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }

  virtual ~QueryGetServicesDescription() {}

  virtual const char* getQueryName() const override;

  void findServicesDescription(std::vector<ServicesDescription*>& servicesDescription,
                               const ServiceGroupDescription& value);

  static void initialize();

  const QueryGetServicesDescription& operator=(const QueryGetServicesDescription& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }

  const QueryGetServicesDescription& operator=(const std::string& Another)
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

class QueryGetServicesDescriptionHistorical : public QueryGetServicesDescription
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;

  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetServicesDescriptionHistorical(DBAdapter* dbAdapt)
    : QueryGetServicesDescription(dbAdapt, _baseSQL)
  {
  }

  virtual ~QueryGetServicesDescriptionHistorical() {}

  virtual const char* getQueryName() const override;

  void findServicesDescription(std::vector<ServicesDescription*>& servicesDescription,
                               const ServiceGroupDescription& value,
                               const DateTime& startDate,
                               const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
} // tse

