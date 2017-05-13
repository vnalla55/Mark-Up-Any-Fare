//----------------------------------------------------------------------------
//          File:           QueryGetMaxStayRestriction.h
//          Description:    QueryGetMaxStayRestriction
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
#include "DBAccess/MaxStayRestriction.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetMaxStayRestriction : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMaxStayRestriction(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetMaxStayRestriction(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetMaxStayRestriction() {};
  virtual const char* getQueryName() const override;

  void findMaxStayRestriction(std::vector<tse::MaxStayRestriction*>& lstMSR,
                              VendorCode& vendor,
                              int itemNo);

  static void initialize();

  const QueryGetMaxStayRestriction& operator=(const QueryGetMaxStayRestriction& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetMaxStayRestriction& operator=(const std::string& Another)
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
}; // class QueryGetMaxStayRestriction

class QueryGetMaxStayRestrictionHistorical : public QueryGetMaxStayRestriction
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMaxStayRestrictionHistorical(DBAdapter* dbAdapt)
    : QueryGetMaxStayRestriction(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetMaxStayRestrictionHistorical() {}
  virtual const char* getQueryName() const override;

  void findMaxStayRestriction(std::vector<tse::MaxStayRestriction*>& lstMSR,
                              VendorCode& vendor,
                              int itemNo,
                              const DateTime& startDate,
                              const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetMaxStayRestrictionHistorical
} // namespace tse

