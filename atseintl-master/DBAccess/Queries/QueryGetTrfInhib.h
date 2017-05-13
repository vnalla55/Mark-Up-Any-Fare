//----------------------------------------------------------------------------
//          File:           QueryGetTrfInhib.h
//          Description:    QueryGetTrfInhib
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
#include "DBAccess/TariffInhibits.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetTrfInhib : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTrfInhib(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetTrfInhib(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetTrfInhib() {}

  virtual const char* getQueryName() const override;

  void findTariffInhibits(std::vector<tse::TariffInhibits*>& lstTI,
                          const VendorCode& vendor,
                          const Indicator tariffCrossRefType,
                          const CarrierCode& carrier,
                          const TariffNumber& fareTariff,
                          const TariffCode& ruleTariffCode);

  static void initialize();

  const QueryGetTrfInhib& operator=(const QueryGetTrfInhib& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTrfInhib& operator=(const std::string& Another)
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
}; // class QueryGetTrfInhib

class QueryGetAllTrfInhib : public QueryGetTrfInhib
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllTrfInhib(DBAdapter* dbAdapt) : QueryGetTrfInhib(dbAdapt, _baseSQL) {}

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::TariffInhibits*>& lstTI) { findAllTariffInhibits(lstTI); }

  void findAllTariffInhibits(std::vector<tse::TariffInhibits*>& lstTI);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllTrfInhib
} // namespace tse

