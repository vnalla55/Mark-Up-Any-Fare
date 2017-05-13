//----------------------------------------------------------------------------
//          File:           QueryGetPfcMultiAirport.h
//          Description:    QueryGetPfcMultiAirport
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
#include "DBAccess/PfcMultiAirport.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetPfcMultiAirport : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetPfcMultiAirport(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetPfcMultiAirport(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetPfcMultiAirport() {}

  virtual const char* getQueryName() const override;

  void findPfcMultiAirport(std::vector<tse::PfcMultiAirport*>& lstMA, const LocCode& loc);

  static void initialize();
  const QueryGetPfcMultiAirport& operator=(const QueryGetPfcMultiAirport& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetPfcMultiAirport& operator=(const std::string& Another)
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
}; // class QueryGetPfcMultiAirport

class QueryGetAllPfcMultiAirport : public QueryGetPfcMultiAirport
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllPfcMultiAirport(DBAdapter* dbAdapt) : QueryGetPfcMultiAirport(dbAdapt, _baseSQL) {}

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::PfcMultiAirport*>& lstMA) { findAllPfcMultiAirport(lstMA); }

  void findAllPfcMultiAirport(std::vector<tse::PfcMultiAirport*>& lstMA);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllPfcMultiAirport

class QueryGetPfcMultiAirportHistorical : public QueryGetPfcMultiAirport
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetPfcMultiAirportHistorical(DBAdapter* dbAdapt) : QueryGetPfcMultiAirport(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetPfcMultiAirportHistorical() {}

  virtual const char* getQueryName() const override;

  void findPfcMultiAirport(std::vector<tse::PfcMultiAirport*>& lstMA,
                           const LocCode& loc,
                           const DateTime& startDate,
                           const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetPfcMultiAirportHistorical

} // namespace tse

