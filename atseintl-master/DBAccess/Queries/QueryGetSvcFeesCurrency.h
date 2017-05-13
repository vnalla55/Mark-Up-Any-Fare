//----------------------------------------------------------------------------
//          File:           QueryGetSvcFeesCurrency.h
//          Description:    QueryGetSvcFeesCurrency.h
//          Created:        11/10/2009
// Authors:
//
//          Updates:
//
//      2009, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "DBAccess/SQLQuery.h"
#include "DBAccess/SvcFeesCurrencyInfo.h"

namespace tse
{

class QueryGetSvcFeesCurrency : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSvcFeesCurrency(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetSvcFeesCurrency(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetSvcFeesCurrency() {};
  virtual const char* getQueryName() const override;

  void findSvcFeesCurrencyInfo(std::vector<tse::SvcFeesCurrencyInfo*>& currency,
                               const VendorCode& vendor,
                               int itemNo);

  static void initialize();

  const QueryGetSvcFeesCurrency& operator=(const QueryGetSvcFeesCurrency& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetSvcFeesCurrency& operator=(const std::string& Another)
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
}; // class QueryGetSvcFeesCurrency

class QueryGetSvcFeesCurrencyHistorical : public QueryGetSvcFeesCurrency
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSvcFeesCurrencyHistorical(DBAdapter* dbAdapt)
    : QueryGetSvcFeesCurrency(dbAdapt, _baseSQL) {};
  virtual ~QueryGetSvcFeesCurrencyHistorical() {}
  virtual const char* getQueryName() const override;

  void findSvcFeesCurrencyInfo(std::vector<tse::SvcFeesCurrencyInfo*>& currency,
                               const VendorCode& vendor,
                               int itemNo,
                               const DateTime& startDate,
                               const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetSvcFeesCurrencyHistorical
} // namespace tse

