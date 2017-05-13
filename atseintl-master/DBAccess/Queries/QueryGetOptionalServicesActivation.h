//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "DBAccess/SQLQuery.h"

namespace tse
{
class OptionalServicesActivationInfo;

class QueryGetOptionalServicesActivation : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetOptionalServicesActivation(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetOptionalServicesActivation(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetOptionalServicesActivation() {}

  virtual const char* getQueryName() const override;

  void
  findOptionalServicesActivationInfo(std::vector<OptionalServicesActivationInfo*>& optSrvActivation,
                                     Indicator crs,
                                     const UserApplCode& userCode,
                                     const std::string& application);

  static void initialize();

  const QueryGetOptionalServicesActivation&
  operator=(const QueryGetOptionalServicesActivation& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }

  const QueryGetOptionalServicesActivation& operator=(const std::string& Another)
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

class QueryGetOptionalServicesActivationHistorical : public QueryGetOptionalServicesActivation
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetOptionalServicesActivationHistorical(DBAdapter* dbAdapt)
    : QueryGetOptionalServicesActivation(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetOptionalServicesActivationHistorical() {}
  virtual const char* getQueryName() const override;

  void
  findOptionalServicesActivationInfo(std::vector<OptionalServicesActivationInfo*>& optSrvActivation,
                                     Indicator crs,
                                     const UserApplCode& userCode,
                                     const std::string& application,
                                     const DateTime& startDate,
                                     const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
} // tse

