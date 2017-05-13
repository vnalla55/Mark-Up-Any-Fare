//----------------------------------------------------------------------------
//          File:           QueryGetMinStayRestriction.h
//          Description:    QueryGetMinStayRestriction
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
#include "DBAccess/MinStayRestriction.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetMinStayRestriction : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMinStayRestriction(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetMinStayRestriction(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetMinStayRestriction() {};
  virtual const char* getQueryName() const override;

  void findMinStayRestriction(std::vector<tse::MinStayRestriction*>& lstMSR,
                              VendorCode& vendor,
                              int itemNo);

  static void initialize();

  const QueryGetMinStayRestriction& operator=(const QueryGetMinStayRestriction& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetMinStayRestriction& operator=(const std::string& Another)
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
}; // class QueryGetMinStayRestriction

class QueryGetMinStayRestrictionHistorical : public QueryGetMinStayRestriction
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMinStayRestrictionHistorical(DBAdapter* dbAdapt)
    : QueryGetMinStayRestriction(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetMinStayRestrictionHistorical() {}
  virtual const char* getQueryName() const override;

  void findMinStayRestriction(std::vector<tse::MinStayRestriction*>& lstMSR,
                              VendorCode& vendor,
                              int itemNo,
                              const DateTime& startDate,
                              const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetMinStayRestrictionHistorical
} // namespace tse

