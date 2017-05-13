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
class MerchCarrierPreferenceInfo;

class QueryGetMerchCarrierPreference : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMerchCarrierPreference(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetMerchCarrierPreference(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetMerchCarrierPreference() {}

  virtual const char* getQueryName() const override;

  void
  findMerchCarrierPreferenceInfo(std::vector<MerchCarrierPreferenceInfo*>& merchCarrierPreferences,
                                 const CarrierCode& carrier,
                                 const ServiceGroup& groupCode);

  static void initialize();

  const QueryGetMerchCarrierPreference& operator=(const QueryGetMerchCarrierPreference& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }

  const QueryGetMerchCarrierPreference& operator=(const std::string& Another)
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

class QueryGetMerchCarrierPreferenceHistorical : public QueryGetMerchCarrierPreference
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMerchCarrierPreferenceHistorical(DBAdapter* dbAdapt)
    : QueryGetMerchCarrierPreference(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetMerchCarrierPreferenceHistorical() {}
  virtual const char* getQueryName() const override;

  void
  findMerchCarrierPreferenceInfo(std::vector<MerchCarrierPreferenceInfo*>& merchCarrierPreferences,
                                 const CarrierCode& carrier,
                                 const ServiceGroup& groupCode,
                                 const DateTime& startDate,
                                 const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
} // tse

