//----------------------------------------------------------------------------
//          File:           QueryGetTaxSegAbsorb.h
//          Description:    QueryGetTaxSegAbsorb
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
#include "DBAccess/TaxSegAbsorb.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetTaxSegAbsorb : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxSegAbsorb(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetTaxSegAbsorb(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetTaxSegAbsorb() {}

  virtual const char* getQueryName() const override;

  void findTaxSegAbsorb(std::vector<tse::TaxSegAbsorb*>& lstTSA, const CarrierCode& cxr);

  static void initialize();

  const QueryGetTaxSegAbsorb& operator=(const QueryGetTaxSegAbsorb& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTaxSegAbsorb& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  }

  static int checkFlightWildCard(const char* fltStr);
  static int stringToInteger(const char* stringVal, int lineNumber);

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetTaxSegAbsorb

class QueryGetAllTaxSegAbsorb : public QueryGetTaxSegAbsorb
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllTaxSegAbsorb(DBAdapter* dbAdapt) : QueryGetTaxSegAbsorb(dbAdapt, _baseSQL) {}

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::TaxSegAbsorb*>& lstTSA) { findAllTaxSegAbsorb(lstTSA); }

  void findAllTaxSegAbsorb(std::vector<tse::TaxSegAbsorb*>& lstTSA);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllTaxSegAbsorb

class QueryGetTaxSegAbsorbHistorical : public QueryGetTaxSegAbsorb
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxSegAbsorbHistorical(DBAdapter* dbAdapt) : QueryGetTaxSegAbsorb(dbAdapt, _baseSQL) {}
  virtual ~QueryGetTaxSegAbsorbHistorical() {}

  virtual const char* getQueryName() const override;

  void findTaxSegAbsorb(std::vector<tse::TaxSegAbsorb*>& lstTSA,
                        const CarrierCode& cxr,
                        const DateTime& startDate,
                        const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetTaxSegAbsorbHistorical

} // namespace tse

