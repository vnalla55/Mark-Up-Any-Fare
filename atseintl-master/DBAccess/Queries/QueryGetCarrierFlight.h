//----------------------------------------------------------------------------
//          File:           QueryGetCarrierFlight.h
//          Description:    QueryGetCarrierFlight
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
#include "DBAccess/CarrierFlight.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetCarrierFlight : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCarrierFlight(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetCarrierFlight(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetCarrierFlight() {};

  virtual const char* getQueryName() const override;

  void findCarrierFlight(std::vector<tse::CarrierFlight*>& lstCF, VendorCode& vendor, int itemNo);

  static void initialize();

  const QueryGetCarrierFlight& operator=(const QueryGetCarrierFlight& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetCarrierFlight& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  };

  static int checkFlightWildCard(const char* fltStr);
  static int stringToInteger(const char* stringVal, int lineNumber);

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetCarrierFlight

class QueryGetCarrierFlightHistorical : public QueryGetCarrierFlight
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCarrierFlightHistorical(DBAdapter* dbAdapt) : QueryGetCarrierFlight(dbAdapt, _baseSQL) {}
  virtual ~QueryGetCarrierFlightHistorical() {}

  virtual const char* getQueryName() const override;

  void findCarrierFlight(std::vector<tse::CarrierFlight*>& lstCF,
                         VendorCode& vendor,
                         int itemNo,
                         const DateTime& startDate,
                         const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetCarrierFlightHistorical
} // namespace tse

