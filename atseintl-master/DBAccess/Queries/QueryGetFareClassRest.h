//----------------------------------------------------------------------------
//          File:           QueryGetFareClassRest.h
//          Description:    QueryGetFareClassRest
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
#include "DBAccess/FareClassRestRule.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetFareClassRest : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareClassRest(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetFareClassRest(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetFareClassRest() {};

  virtual const char* getQueryName() const override;

  void findFareClassRestRule(std::vector<tse::FareClassRestRule*>& fareClassRests,
                             const VendorCode& vendor,
                             int itemNumber);

  static void initialize();

  const QueryGetFareClassRest& operator=(const QueryGetFareClassRest& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetFareClassRest& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  };

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetFareClassRest

class QueryGetFareClassRestHistorical : public QueryGetFareClassRest
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareClassRestHistorical(DBAdapter* dbAdapt) : QueryGetFareClassRest(dbAdapt, _baseSQL) {}
  virtual ~QueryGetFareClassRestHistorical() {}

  virtual const char* getQueryName() const override;

  void findFareClassRestRule(std::vector<tse::FareClassRestRule*>& fareClassRests,
                             const VendorCode& vendor,
                             int itemNumber,
                             const DateTime& startDate,
                             const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetFareClassRestHistorical
} // namespace tse
