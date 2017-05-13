//----------------------------------------------------------------------------
//          File:           QueryGetBkgException.h
//          Description:    QueryGetBkgException
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
#include "DBAccess/BookingCodeExceptionSegment.h"
#include "DBAccess/BookingCodeExceptionSequence.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetBkgException : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetBkgException(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetBkgException(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetBkgException() {};

  virtual const char* getQueryName() const override;

  void findBkgExcpt(std::vector<tse::BookingCodeExceptionSequence*>& bkgExcpts,
                    const VendorCode& vendor,
                    int itemNo);

  static void initialize();

  const QueryGetBkgException& operator=(const QueryGetBkgException& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetBkgException& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  };
  static int mapYear(const char* s);
  static int stringToInteger(const char* stringVal, int lineNumber);

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetBkgException

class QueryGetBkgExceptionHistorical : public QueryGetBkgException
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetBkgExceptionHistorical(DBAdapter* dbAdapt) : QueryGetBkgException(dbAdapt, _baseSQL) {}
  virtual ~QueryGetBkgExceptionHistorical() {}

  virtual const char* getQueryName() const override;

  void findBkgExcpt(std::vector<tse::BookingCodeExceptionSequence*>& bkgExcpts,
                    const VendorCode& vendor,
                    int itemNo,
                    const DateTime& startDate,
                    const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetBkgExceptionHistorical
} // namespace tse

