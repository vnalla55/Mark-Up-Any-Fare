//----------------------------------------------------------------------------
//          File:           QueryGetBlackout.h
//          Description:    QueryGetBlackout
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
#include "DBAccess/BlackoutInfo.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetBlackout : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetBlackout(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetBlackout(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetBlackout() {};

  virtual const char* getQueryName() const override;

  void
  findBlackoutInfo(std::vector<BlackoutInfo*>& blackout, const VendorCode& vendor, int itemNumber);

  static void initialize();

  const QueryGetBlackout& operator=(const QueryGetBlackout& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetBlackout& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  };
  static int stringToInteger(const char* stringVal, int lineNumber);

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetBlackout

class QueryGetBlackoutHistorical : public QueryGetBlackout
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetBlackoutHistorical(DBAdapter* dbAdapt) : QueryGetBlackout(dbAdapt, _baseSQL) {}
  virtual ~QueryGetBlackoutHistorical() {}

  virtual const char* getQueryName() const override;

  void findBlackoutInfo(std::vector<BlackoutInfo*>& blackout,
                        const VendorCode& vendor,
                        int itemNumber,
                        const DateTime& startDate,
                        const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetBlackoutHistorical
} // namespace tse

