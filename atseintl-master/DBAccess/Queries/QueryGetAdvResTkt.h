//----------------------------------------------------------------------------
//          File:           QueryGetAdvResTkt.h
//          Description:    QueryGetAdvResTkt
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
#include "DBAccess/AdvResTktInfo.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetAdvResTkt : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAdvResTkt(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetAdvResTkt(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetAdvResTkt() {};

  virtual const char* getQueryName() const override;

  void
  findAdvResTktInfo(std::vector<AdvResTktInfo*>& advres, const VendorCode& vendor, int itemNumber);

  static void initialize();

  const QueryGetAdvResTkt& operator=(const QueryGetAdvResTkt& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetAdvResTkt& operator=(const std::string& Another)
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
}; // class QueryGetAdvResTkt

class QueryGetAdvResTktHistorical : public QueryGetAdvResTkt
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAdvResTktHistorical(DBAdapter* dbAdapt) : QueryGetAdvResTkt(dbAdapt, _baseSQL) {}
  virtual ~QueryGetAdvResTktHistorical() {}

  virtual const char* getQueryName() const override;

  void findAdvResTktInfo(std::vector<AdvResTktInfo*>& advres,
                         const VendorCode& vendor,
                         int itemNumber,
                         const DateTime& startDate,
                         const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAdvResTktHistorical
} // namespace tse

