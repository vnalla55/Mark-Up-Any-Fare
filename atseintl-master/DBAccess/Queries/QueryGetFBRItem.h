//----------------------------------------------------------------------------
//          File:           QueryGetFBRItem.h
//          Description:    QueryGetFBRItem
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
#include "DBAccess/FareByRuleItemInfo.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetFBRItem : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFBRItem(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetFBRItem(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetFBRItem() {};
  virtual const char* getQueryName() const override;

  void findFareByRuleItemInfo(std::vector<FareByRuleItemInfo*>& fbrs,
                              const VendorCode& vendor,
                              int itemNumber);

  static void initialize();

  const QueryGetFBRItem& operator=(const QueryGetFBRItem& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetFBRItem& operator=(const std::string& Another)
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
}; // class QueryGetFBRItem

class QueryGetFBRItemHistorical : public QueryGetFBRItem
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFBRItemHistorical(DBAdapter* dbAdapt) : QueryGetFBRItem(dbAdapt, _baseSQL) {}
  virtual ~QueryGetFBRItemHistorical() {}
  virtual const char* getQueryName() const override;

  void findFareByRuleItemInfo(std::vector<FareByRuleItemInfo*>& fbrs,
                              const VendorCode& vendor,
                              int itemNumber,
                              const DateTime& startDate,
                              const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetFBRItemHistorical
} // namespace tse

