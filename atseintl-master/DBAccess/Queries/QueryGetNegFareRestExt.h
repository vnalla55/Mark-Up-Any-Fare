//----------------------------------------------------------------------------
//          File:           QueryGetNegFareRestExt.h
//          Description:    QueryGetNegFareRestExt
//          Created:        9/9/2010
// Authors:         Artur Krezel
//
//          Updates:
//
//     � 2010, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "DBAccess/NegFareRestExt.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetNegFareRestExt : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetNegFareRestExt(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetNegFareRestExt(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetNegFareRestExt() {};
  virtual const char* getQueryName() const override;

  void
  findNegFareRestExt(std::vector<tse::NegFareRestExt*>& lstNFR, VendorCode& vendor, int itemNo);

  static void initialize();

  const QueryGetNegFareRestExt& operator=(const QueryGetNegFareRestExt& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetNegFareRestExt& operator=(const std::string& Another)
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
}; // class QueryGetNegFareRestExt

class QueryGetNegFareRestExtHistorical : public QueryGetNegFareRestExt
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetNegFareRestExtHistorical(DBAdapter* dbAdapt) : QueryGetNegFareRestExt(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetNegFareRestExtHistorical() {}
  virtual const char* getQueryName() const override;

  void findNegFareRestExt(std::vector<tse::NegFareRestExt*>& lstNFR,
                          VendorCode& vendor,
                          int itemNo,
                          const DateTime& startDate,
                          const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetNegFareRestExtHistorical
} // namespace tse

