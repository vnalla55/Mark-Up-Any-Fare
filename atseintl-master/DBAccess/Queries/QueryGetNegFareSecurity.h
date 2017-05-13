//----------------------------------------------------------------------------
//          File:           QueryGetNegFareSecurity.h
//          Description:    QueryGetNegFareSecurity
//          Created:        3/2/2006
// Authors:         Mike Lillis
//
//          Updates:
//
//     � 2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "DBAccess/NegFareSecurityInfo.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetNegFareSecurity : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetNegFareSecurity(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetNegFareSecurity(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetNegFareSecurity() {};
  virtual const char* getQueryName() const override;

  void findNegFareSecurity(std::vector<NegFareSecurityInfo*>& negFareSec,
                           VendorCode& vendor,
                           int itemNumber);

  static void initialize();

  const QueryGetNegFareSecurity& operator=(const QueryGetNegFareSecurity& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetNegFareSecurity& operator=(const std::string& Another)
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
}; // class QueryGetNegFareSecurity

class QueryGetNegFareSecurityHistorical : public QueryGetNegFareSecurity
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetNegFareSecurityHistorical(DBAdapter* dbAdapt) : QueryGetNegFareSecurity(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetNegFareSecurityHistorical() {}
  virtual const char* getQueryName() const override;

  void findNegFareSecurity(std::vector<NegFareSecurityInfo*>& negFareSec,
                           VendorCode& vendor,
                           int itemNumber,
                           const DateTime& startDate,
                           const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetNegFareSecurityHistorical
} // namespace tse

