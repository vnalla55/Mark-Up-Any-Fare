//----------------------------------------------------------------------------
//     2012, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "DBAccess/SQLQuery.h"
#include "DBAccess/USDotCarrier.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetUSDotCarrier : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetUSDotCarrier(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetUSDotCarrier(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetUSDotCarrier() {};

  virtual const char* getQueryName() const override;

  void findCarrier(std::vector<tse::USDotCarrier*>& lstCF, const CarrierCode& vendor);

  static void initialize();

  const QueryGetUSDotCarrier& operator=(const QueryGetUSDotCarrier& another)
  {
    if (this != &another)
      *((SQLQuery*)this) = (SQLQuery&)another;

    return *this;
  };

  const QueryGetUSDotCarrier& operator=(const std::string& another)
  {
    if (this != &another)
      *((SQLQuery*)this) = another;

    return *this;
  };

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetUSDotCarrier

class QueryGetUSDotCarrierHistorical : public QueryGetUSDotCarrier
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetUSDotCarrierHistorical(DBAdapter* dbAdapt) : QueryGetUSDotCarrier(dbAdapt, _baseSQL) {}
  virtual ~QueryGetUSDotCarrierHistorical() {}

  virtual const char* getQueryName() const override;

  void findCarrier(std::vector<tse::USDotCarrier*>& lstCF, const CarrierCode& carrier);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetUSDotCarrierHistorical

} // namespace tse

