//----------------------------------------------------------------------------
//          File:           QueryGetCircleTripRuleItem.h
//          Description:    QueryGetCircleTripRuleItem
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
#include "DBAccess/CircleTripRuleItem.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetCircleTripRuleItem : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCircleTripRuleItem(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetCircleTripRuleItem(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetCircleTripRuleItem() {};

  virtual const char* getQueryName() const override;

  void findCircleTripRuleItem(std::vector<tse::CircleTripRuleItem*>& circles,
                              const VendorCode& vendor,
                              int itemNumber);

  static void initialize();

  const QueryGetCircleTripRuleItem& operator=(const QueryGetCircleTripRuleItem& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetCircleTripRuleItem& operator=(const std::string& Another)
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
}; // class QueryGetCircleTripRuleItem

class QueryGetCircleTripRuleItemHistorical : public QueryGetCircleTripRuleItem
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCircleTripRuleItemHistorical(DBAdapter* dbAdapt)
    : QueryGetCircleTripRuleItem(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetCircleTripRuleItemHistorical() {}

  virtual const char* getQueryName() const override;

  void findCircleTripRuleItem(std::vector<tse::CircleTripRuleItem*>& circles,
                              const VendorCode& vendor,
                              int itemNumber,
                              const DateTime& startDate,
                              const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetCircleTripRuleItemHistorical
} // namespace tse

