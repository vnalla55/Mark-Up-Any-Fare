//----------------------------------------------------------------------------
//          File:           QueryGetSvcFeesSecurity.h
//          Description:    QueryGetSvcFeesSecurity.h
//          Created:        3/9/2009
// Authors:
//
//          Updates:
//
//      2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "DBAccess/SQLQuery.h"
#include "DBAccess/SvcFeesSecurityInfo.h"

namespace tse
{

class QueryGetSvcFeesSecurity : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSvcFeesSecurity(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetSvcFeesSecurity(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetSvcFeesSecurity() {};
  virtual const char* getQueryName() const override;

  void findSvcFeesSecurityInfo(std::vector<tse::SvcFeesSecurityInfo*>& sec,
                               const VendorCode& vendor,
                               int itemNo);

  static void initialize();

  const QueryGetSvcFeesSecurity& operator=(const QueryGetSvcFeesSecurity& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetSvcFeesSecurity& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  };

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetSvcFeesSecurity

class QueryGetSvcFeesSecurityHistorical : public QueryGetSvcFeesSecurity
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSvcFeesSecurityHistorical(DBAdapter* dbAdapt)
    : QueryGetSvcFeesSecurity(dbAdapt, _baseSQL) {};
  virtual ~QueryGetSvcFeesSecurityHistorical() {}
  virtual const char* getQueryName() const override;

  void findSvcFeesSecurityInfo(std::vector<tse::SvcFeesSecurityInfo*>& sec,
                               const VendorCode& vendor,
                               int itemNo,
                               const DateTime& startDate,
                               const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetSvcFeesSecurityHistorical
} // namespace tse

