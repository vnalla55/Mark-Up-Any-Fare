//----------------------------------------------------------------------------
//          File:           QueryGetSvcFeesFeature.h
//          Description:    QueryGetSvcFeesFeature
//          Created:        04/08/2013
//
//     Copyright 2013, Sabre Inc. All rights reserved. This software/documentation is
//     confidential and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "Common/Logger.h"
#include "DBAccess/SQLQuery.h"
#include "DBAccess/SvcFeesFeatureInfo.h"

namespace tse
{
class QueryGetSvcFeesFeature : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSvcFeesFeature(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetSvcFeesFeature(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetSvcFeesFeature() {}
  virtual const char* getQueryName() const override;

  void findSvcFeesFeature(std::vector<SvcFeesFeatureInfo*>& lst,
                          const VendorCode& vendor,
                          long long iemNo);

  static void initialize();

  const QueryGetSvcFeesFeature& operator=(const QueryGetSvcFeesFeature& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetSvcFeesFeature& operator=(const std::string& Another)
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

class QueryGetSvcFeesFeatureHistorical : public QueryGetSvcFeesFeature
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSvcFeesFeatureHistorical(DBAdapter* dbAdapt) : QueryGetSvcFeesFeature(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetSvcFeesFeatureHistorical() {}
  virtual const char* getQueryName() const override;

  void findSvcFeesFeature(std::vector<SvcFeesFeatureInfo*>& lst,
                          const VendorCode& vendor,
                          long long itemNo,
                          const DateTime& startDate,
                          const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

class QueryGetAllSvcFeesFeature : public QueryGetSvcFeesFeature
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllSvcFeesFeature(DBAdapter* dbAdapt) : QueryGetSvcFeesFeature(dbAdapt, _baseSQL) {}
  virtual ~QueryGetAllSvcFeesFeature() {}
  virtual const char* getQueryName() const override;

  void execute(std::vector<SvcFeesFeatureInfo*>& lst) { findAllSvcFeesFeature(lst); }
  void findAllSvcFeesFeature(std::vector<SvcFeesFeatureInfo*>& lst);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
} // tse

