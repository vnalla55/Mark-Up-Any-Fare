//----------------------------------------------------------------------------
//          File:           QueryGetYQYRFees.h
//          Description:    QueryGetYQYRFees
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
#include "DBAccess/YQYRFees.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetYQYRFees : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetYQYRFees(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetYQYRFees(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetYQYRFees() {}

  virtual const char* getQueryName() const override;

  void findYQYRFees(std::vector<tse::YQYRFees*>& yqyrFees, CarrierCode& carrier);

  static void initialize();
  const QueryGetYQYRFees& operator=(const QueryGetYQYRFees& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetYQYRFees& operator=(const std::string& Another)
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
}; // class QueryGetYQYRFees

class QueryGetAllYQYRFees : public QueryGetYQYRFees
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllYQYRFees(DBAdapter* dbAdapt) : QueryGetYQYRFees(dbAdapt, _baseSQL) {}

  virtual const char* getQueryName() const override;
  void execute(std::vector<tse::YQYRFees*>& yqyrFees) { findAllYQYRFees(yqyrFees); }

  void findAllYQYRFees(std::vector<tse::YQYRFees*>& yqyrFees);
  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllYQYRFees

class QueryGetYQYRFeesHistorical : public QueryGetYQYRFees
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetYQYRFeesHistorical(DBAdapter* dbAdapt) : QueryGetYQYRFees(dbAdapt, _baseSQL) {}
  virtual ~QueryGetYQYRFeesHistorical() {}
  virtual const char* getQueryName() const override;

  void findYQYRFees(std::vector<tse::YQYRFees*>& yqyrFees,
                    CarrierCode& carrier,
                    const DateTime& startDate,
                    const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetYQYRFeesHistorical

} // namespace tse

