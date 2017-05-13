//----------------------------------------------------------------------------
//          File:           QueryGetPfcAbsorb.h
//          Description:    QueryGetPfcAbsorb
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
#include "DBAccess/PfcAbsorb.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetPfcAbsorb : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetPfcAbsorb(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetPfcAbsorb(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetPfcAbsorb() {}

  virtual const char* getQueryName() const override;

  void findPfcAbsorb(std::vector<tse::PfcAbsorb*>& lstAbs, LocCode& pfcAirpt, CarrierCode& locCxr);

  static void initialize();
  const QueryGetPfcAbsorb& operator=(const QueryGetPfcAbsorb& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetPfcAbsorb& operator=(const std::string& Another)
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
}; // class QueryGetPfcAbsorb

class QueryGetAllPfcAbsorb : public QueryGetPfcAbsorb
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllPfcAbsorb(DBAdapter* dbAdapt) : QueryGetPfcAbsorb(dbAdapt, _baseSQL) {}

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::PfcAbsorb*>& lstAbsorb) { findAllPfcAbsorb(lstAbsorb); }

  void findAllPfcAbsorb(std::vector<tse::PfcAbsorb*>& lstAbsorb);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllPfcAbsorb

class QueryGetPfcAbsorbHistorical : public QueryGetPfcAbsorb
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetPfcAbsorbHistorical(DBAdapter* dbAdapt) : QueryGetPfcAbsorb(dbAdapt, _baseSQL) {}
  QueryGetPfcAbsorbHistorical(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : QueryGetPfcAbsorb(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetPfcAbsorbHistorical() {}

  virtual const char* getQueryName() const override;

  void findPfcAbsorbHistorical(std::vector<tse::PfcAbsorb*>& lstAbs,
                               LocCode& pfcAirpt,
                               CarrierCode& locCxr,
                               const DateTime& startDate,
                               const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetPfcAbsorb

} // namespace tse
