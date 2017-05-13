//----------------------------------------------------------------------------
//          File:           QueryGetTaxAkHiFactor.h
//          Description:    QueryGetTaxAkHiFactor
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
#include "DBAccess/TaxAkHiFactor.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetOneTaxAkHiFactor : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetOneTaxAkHiFactor(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetOneTaxAkHiFactor(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetOneTaxAkHiFactor() {}

  virtual const char* getQueryName() const override;

  void findTaxAkHiFactor(std::vector<tse::TaxAkHiFactor*>& lstTAHF, LocCode& city);

  static void initialize();

  const QueryGetOneTaxAkHiFactor& operator=(const QueryGetOneTaxAkHiFactor& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetOneTaxAkHiFactor& operator=(const std::string& Another)
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
}; // class QueryGetOneTaxAkHiFactor

class QueryGetAllTaxAkHiFactor : public QueryGetOneTaxAkHiFactor
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllTaxAkHiFactor(DBAdapter* dbAdapt) : QueryGetOneTaxAkHiFactor(dbAdapt, _baseSQL) {}

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::TaxAkHiFactor*>& lstTAHF) { findAllTaxAkHiFactor(lstTAHF); }

  void findAllTaxAkHiFactor(std::vector<tse::TaxAkHiFactor*>& lstTAHF);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllTaxAkHiFactor

class QueryGetOneTaxAkHiFactorHistorical : public QueryGetOneTaxAkHiFactor
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetOneTaxAkHiFactorHistorical(DBAdapter* dbAdapt)
    : QueryGetOneTaxAkHiFactor(dbAdapt, _baseSQL)
  {
  }
  QueryGetOneTaxAkHiFactorHistorical(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : QueryGetOneTaxAkHiFactor(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetOneTaxAkHiFactorHistorical() {}

  virtual const char* getQueryName() const override;

  void findTaxAkHiFactorHistorical(std::vector<tse::TaxAkHiFactor*>& lstTAHF,
                                   LocCode& city,
                                   const DateTime& startDate,
                                   const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

} // namespace tse

