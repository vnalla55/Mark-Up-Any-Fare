//----------------------------------------------------------------------------
//          File:           QueryGetYQYRFeesNonConcur.h
//          Description:    QueryGetYQYRFeesNonConcur
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
#include "DBAccess/YQYRFeesNonConcur.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetYQYRFeesNonConcur : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetYQYRFeesNonConcur(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetYQYRFeesNonConcur(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetYQYRFeesNonConcur() {}

  virtual const char* getQueryName() const override;
  void findYQYRFeesNonConcur(std::vector<tse::YQYRFeesNonConcur*>& yqyrFNCs, CarrierCode& carrier);
  static void initialize();
  const QueryGetYQYRFeesNonConcur& operator=(const QueryGetYQYRFeesNonConcur& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetYQYRFeesNonConcur& operator=(const std::string& Another)
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
}; // class QueryGetYQYRFeesNonConcur

class QueryGetAllYQYRFeesNonConcur : public QueryGetYQYRFeesNonConcur
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllYQYRFeesNonConcur(DBAdapter* dbAdapt) : QueryGetYQYRFeesNonConcur(dbAdapt, _baseSQL) {}

  virtual const char* getQueryName() const override;
  void execute(std::vector<tse::YQYRFeesNonConcur*>& yqyrFNCs)
  {
    findAllYQYRFeesNonConcur(yqyrFNCs);
  }

  void findAllYQYRFeesNonConcur(std::vector<tse::YQYRFeesNonConcur*>& yqyrFNCs);
  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllYQYRFeesNonConcur

class QueryGetYQYRFeesNonConcurHistorical : public QueryGetYQYRFeesNonConcur
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetYQYRFeesNonConcurHistorical(DBAdapter* dbAdapt)
    : QueryGetYQYRFeesNonConcur(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetYQYRFeesNonConcurHistorical() {}

  virtual const char* getQueryName() const override;

  void findYQYRFeesNonConcur(std::vector<tse::YQYRFeesNonConcur*>& yqyrFNCs,
                             CarrierCode& carrier,
                             const DateTime& startDate,
                             const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetYQYRFeesNonConcurHistorical

} // namespace tse

