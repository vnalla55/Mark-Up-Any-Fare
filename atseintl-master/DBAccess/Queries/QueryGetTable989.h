//----------------------------------------------------------------------------
//          File:           QueryGetTable989.h
//          Description:    QueryGetTable989
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
#include "DBAccess/BaseFareRule.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetTable989 : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTable989(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetTable989(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetTable989() {};

  virtual const char* getQueryName() const override;

  void findBaseFareRule(std::vector<const tse::BaseFareRule*>& table989s,
                        VendorCode& vendor,
                        int itemNo);

  static void initialize();

  const QueryGetTable989& operator=(const QueryGetTable989& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetTable989& operator=(const std::string& Another)
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
}; // class QueryGetTable989

class QueryGetTable989Historical : public QueryGetTable989
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTable989Historical(DBAdapter* dbAdapt) : QueryGetTable989(dbAdapt, _baseSQL) {}
  virtual ~QueryGetTable989Historical() {}

  virtual const char* getQueryName() const override;

  void findBaseFareRule(std::vector<const tse::BaseFareRule*>& table989s,
                        VendorCode& vendor,
                        int itemNo,
                        const DateTime& startDate,
                        const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetTable989Historical
} // namespace tse

