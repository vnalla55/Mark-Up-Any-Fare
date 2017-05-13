//----------------------------------------------------------------------------
//          File:           QueryGetVisitAnotherCountry.h
//          Description:    QueryGetVisitAnotherCountry
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
#include "DBAccess/VisitAnotherCountry.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetVisitAnotherCountry : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetVisitAnotherCountry(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetVisitAnotherCountry(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetVisitAnotherCountry() {}
  virtual const char* getQueryName() const override;

  void findVisitAnotherCountry(std::vector<tse::VisitAnotherCountry*>& visits,
                               const VendorCode& vendor,
                               int itemNo);

  static void initialize();

  const QueryGetVisitAnotherCountry& operator=(const QueryGetVisitAnotherCountry& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetVisitAnotherCountry& operator=(const std::string& Another)
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
}; // class QueryGetVisitAnotherCountry

class QueryGetVisitAnotherCountryHistorical : public QueryGetVisitAnotherCountry
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetVisitAnotherCountryHistorical(DBAdapter* dbAdapt)
    : QueryGetVisitAnotherCountry(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetVisitAnotherCountryHistorical() {}
  virtual const char* getQueryName() const override;

  void findVisitAnotherCountry(std::vector<tse::VisitAnotherCountry*>& visits,
                               const VendorCode& vendor,
                               int itemNo,
                               const DateTime& startDate,
                               const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetVisitAnotherCountryHistorical
} // namespace tse

