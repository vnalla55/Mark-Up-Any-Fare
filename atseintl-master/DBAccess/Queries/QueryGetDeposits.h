//----------------------------------------------------------------------------
//          File:           QueryGetDeposits.h
//          Description:    QueryGetDeposits
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
#include "DBAccess/Deposits.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetDeposits : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetDeposits(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetDeposits(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetDeposits() {};

  virtual const char* getQueryName() const override;

  void findDeposits(std::vector<tse::Deposits*>& deposits, const VendorCode& vendor, int itemNo);

  static void initialize();

  const QueryGetDeposits& operator=(const QueryGetDeposits& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetDeposits& operator=(const std::string& Another)
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
}; // class QueryGetDeposits

class QueryGetDepositsHistorical : public QueryGetDeposits
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetDepositsHistorical(DBAdapter* dbAdapt) : QueryGetDeposits(dbAdapt, _baseSQL) {}
  virtual ~QueryGetDepositsHistorical() {}

  virtual const char* getQueryName() const override;

  void findDeposits(std::vector<tse::Deposits*>& deposits,
                    const VendorCode& vendor,
                    int itemNo,
                    const DateTime& startDate,
                    const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetDepositsHistorical
} // namespace tse

