//----------------------------------------------------------------------------
//          File:           QueryGetNegFareCalcInfo.h
//          Description:    QueryGetNegFareCalcInfo
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
#include "DBAccess/NegFareCalcInfo.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetNegFareCalcInfo : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetNegFareCalcInfo(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetNegFareCalcInfo(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetNegFareCalcInfo() {};
  virtual const char* getQueryName() const override;

  void
  findNegFareCalcInfo(std::vector<tse::NegFareCalcInfo*>& lstNFC, VendorCode& vendor, int itemNo);

  static void initialize();

  const QueryGetNegFareCalcInfo& operator=(const QueryGetNegFareCalcInfo& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetNegFareCalcInfo& operator=(const std::string& Another)
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
}; // class QueryGetNegFareCalcInfo

class QueryGetNegFareCalcInfoHistorical : public QueryGetNegFareCalcInfo
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetNegFareCalcInfoHistorical(DBAdapter* dbAdapt) : QueryGetNegFareCalcInfo(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetNegFareCalcInfoHistorical() {}
  virtual const char* getQueryName() const override;

  void findNegFareCalcInfo(std::vector<tse::NegFareCalcInfo*>& lstNFC,
                           VendorCode& vendor,
                           int itemNo,
                           const DateTime& startDate,
                           const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetNegFareCalcInfoHistorical
} // namespace tse

