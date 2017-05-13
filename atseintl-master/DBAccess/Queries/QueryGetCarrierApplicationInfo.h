//----------------------------------------------------------------------------
//          File:           QueryGetCarrierApplicationInfo.h
//          Description:    QueryGetCarrierApplicationInfo
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
#include "DBAccess/CarrierApplicationInfo.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetCarrierApplicationInfo : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCarrierApplicationInfo(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetCarrierApplicationInfo(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetCarrierApplicationInfo() {};

  virtual const char* getQueryName() const override;

  void findCarrierApplicationInfo(std::vector<tse::CarrierApplicationInfo*>& lstCAI,
                                  VendorCode& vendor,
                                  int itemNo);

  static void initialize();

  const QueryGetCarrierApplicationInfo& operator=(const QueryGetCarrierApplicationInfo& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetCarrierApplicationInfo& operator=(const std::string& Another)
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
}; // class QueryGetCarrierApplicationInfo

class QueryGetCarrierApplicationInfoHistorical : public QueryGetCarrierApplicationInfo
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCarrierApplicationInfoHistorical(DBAdapter* dbAdapt)
    : QueryGetCarrierApplicationInfo(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetCarrierApplicationInfoHistorical() {}

  virtual const char* getQueryName() const override;

  void findCarrierApplicationInfo(std::vector<tse::CarrierApplicationInfo*>& lstCAI,
                                  VendorCode& vendor,
                                  int itemNo,
                                  const DateTime& startDate,
                                  const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetCarrierApplicationInfoHistorical
} // namespace tse

