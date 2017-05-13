//----------------------------------------------------------------------------
//          File:           QueryGetEndOnEnd.h
//          Description:    QueryGetEndOnEnd
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
#include "DBAccess/EndOnEnd.h"
#include "DBAccess/EndOnEndSegment.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetEndOnEnd : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetEndOnEnd(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetEndOnEnd(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetEndOnEnd() {};
  virtual const char* getQueryName() const override;

  void findEndOnEnd(std::vector<const tse::EndOnEnd*>& endOnEnds,
                    const VendorCode& vendor,
                    int itemNumber);

  static void initialize();

  const QueryGetEndOnEnd& operator=(const QueryGetEndOnEnd& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetEndOnEnd& operator=(const std::string& Another)
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
}; // class QueryGetEndOnEnd

class QueryGetEndOnEndHistorical : public QueryGetEndOnEnd
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetEndOnEndHistorical(DBAdapter* dbAdapt) : QueryGetEndOnEnd(dbAdapt, _baseSQL) {}
  virtual ~QueryGetEndOnEndHistorical() {}
  virtual const char* getQueryName() const override;

  void findEndOnEnd(std::vector<const tse::EndOnEnd*>& endOnEnds,
                    const VendorCode& vendor,
                    int itemNumber,
                    const DateTime& startDate,
                    const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetEndOnEndHistorical
} // namespace tse

