//----------------------------------------------------------------------------
//          File:           QueryGetCarrierComb.h
//          Description:    QueryGetCarrierComb
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
#include "DBAccess/CarrierCombination.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetCarrierComb : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCarrierComb(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetCarrierComb(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetCarrierComb() {};

  virtual const char* getQueryName() const override;

  void findCarrierCombination(std::vector<CarrierCombination*>& carrierCombs,
                              const VendorCode& vendor,
                              int itemNumber);

  static void initialize();

  const QueryGetCarrierComb& operator=(const QueryGetCarrierComb& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetCarrierComb& operator=(const std::string& Another)
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
}; // class QueryGetCarrierComb

class QueryGetCarrierCombHistorical : public QueryGetCarrierComb
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCarrierCombHistorical(DBAdapter* dbAdapt) : QueryGetCarrierComb(dbAdapt, _baseSQL) {}
  virtual ~QueryGetCarrierCombHistorical() {}

  virtual const char* getQueryName() const override;

  void findCarrierCombination(std::vector<CarrierCombination*>& carrierCombs,
                              const VendorCode& vendor,
                              int itemNumber,
                              const DateTime& startDate,
                              const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetCarrierCombHistorical
} // namespace tse

