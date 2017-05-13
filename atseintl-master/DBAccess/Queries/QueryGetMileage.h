//----------------------------------------------------------------------------
//          File:           QueryGetMileage.h
//          Description:    QueryGetMileage
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
#include "DBAccess/Mileage.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetMileage : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMileage(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetMileage(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetMileage() {};

  virtual const char* getQueryName() const override;

  void findMileage(std::vector<tse::Mileage*>& mileages,
                   const LocCode& orig,
                   const LocCode& dest,
                   const Indicator mileType);

  static void initialize();

  const QueryGetMileage& operator=(const QueryGetMileage& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetMileage& operator=(const std::string& Another)
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
}; // class QueryGetMileage

class QueryGetMileageHistorical : public QueryGetMileage
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMileageHistorical(DBAdapter* dbAdapt) : QueryGetMileage(dbAdapt, _baseSQL) {};
  virtual ~QueryGetMileageHistorical() {}
  virtual const char* getQueryName() const override;

  void findMileage(std::vector<tse::Mileage*>& mileages,
                   const LocCode& orig,
                   const LocCode& dest,
                   const Indicator mileType,
                   const DateTime& startDate,
                   const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetMileageHistorical
} // namespace tse

