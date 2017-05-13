//----------------------------------------------------------------------------
//          File:           QueryGetOpenJawRule.h
//          Description:    QueryGetOpenJawRule
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
#include "DBAccess/OpenJawRule.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetOpenJawRule : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetOpenJawRule(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetOpenJawRule(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetOpenJawRule() {};
  virtual const char* getQueryName() const override;

  void
  findOpenJawRule(std::vector<OpenJawRule*>& openJaws, const VendorCode& vendor, int itemNumber);

  static void initialize();

  const QueryGetOpenJawRule& operator=(const QueryGetOpenJawRule& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetOpenJawRule& operator=(const std::string& Another)
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
}; // class QueryGetOpenJawRule

class QueryGetOpenJawRuleHistorical : public QueryGetOpenJawRule
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetOpenJawRuleHistorical(DBAdapter* dbAdapt) : QueryGetOpenJawRule(dbAdapt, _baseSQL) {}
  virtual ~QueryGetOpenJawRuleHistorical() {}
  virtual const char* getQueryName() const override;

  void findOpenJawRule(std::vector<OpenJawRule*>& openJaws,
                       const VendorCode& vendor,
                       int itemNumber,
                       const DateTime& startDate,
                       const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetOpenJawRuleHistorical
} // namespace tse

