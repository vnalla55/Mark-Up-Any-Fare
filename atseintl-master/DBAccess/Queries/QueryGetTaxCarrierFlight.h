//----------------------------------------------------------------------------
//     2010, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "DBAccess/SQLQuery.h"
#include "DBAccess/TaxCarrierFlightInfo.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetTaxCarrierFlight : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxCarrierFlight(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetTaxCarrierFlight(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetTaxCarrierFlight() {};

  virtual const char* getQueryName() const override;

  void
  findCarrierFlight(std::vector<tse::TaxCarrierFlightInfo*>& lstCF, VendorCode& vendor, int itemNo);

  static void initialize();

  const QueryGetTaxCarrierFlight& operator=(const QueryGetTaxCarrierFlight& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetTaxCarrierFlight& operator=(const std::string& Another)
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
}; // class QueryGetTaxCarrierFlight

class QueryGetTaxCarrierFlightHistorical : public QueryGetTaxCarrierFlight
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxCarrierFlightHistorical(DBAdapter* dbAdapt)
    : QueryGetTaxCarrierFlight(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetTaxCarrierFlightHistorical() {}

  virtual const char* getQueryName() const override;

  void findCarrierFlight(std::vector<tse::TaxCarrierFlightInfo*>& lstCF,
                         VendorCode& vendor,
                         int itemNo,
                         const DateTime& startDate,
                         const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetTaxCarrierFlightHistorical
} // namespace tse

