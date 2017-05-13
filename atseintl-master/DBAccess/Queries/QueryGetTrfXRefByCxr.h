//----------------------------------------------------------------------------
//          File:           QueryGetTrfXRefByCxr.h
//          Description:    QueryGetTrfXRefByCxr
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
#include "DBAccess/SQLQuery.h"
#include "DBAccess/TariffCrossRefInfo.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetTrfXRefByCxr : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTrfXRefByCxr(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetTrfXRefByCxr(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetTrfXRefByCxr() {}

  virtual const char* getQueryName() const override;

  void findTariffXRefByCarrier(std::vector<tse::TariffCrossRefInfo*>& tarList,
                               CarrierCode& carrier,
                               VendorCode& vendor,
                               std::string& tariffCrossRefType);

  static void initialize();

  const QueryGetTrfXRefByCxr& operator=(const QueryGetTrfXRefByCxr& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTrfXRefByCxr& operator=(const std::string& Another)
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
}; // class QueryGetTrfXRefByCxr

class QueryGetTrfXRefByCxrHistorical : public QueryGetTrfXRefByCxr
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTrfXRefByCxrHistorical(DBAdapter* dbAdapt) : QueryGetTrfXRefByCxr(dbAdapt, _baseSQL) {}
  virtual ~QueryGetTrfXRefByCxrHistorical() {}

  virtual const char* getQueryName() const override;

  void findTariffXRefByCarrier(std::vector<tse::TariffCrossRefInfo*>& tarList,
                               CarrierCode& carrier,
                               VendorCode& vendor,
                               std::string& tariffCrossRefType,
                               const DateTime& startDate,
                               const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetTrfXRefByCxrHistorical
} // namespace tse
