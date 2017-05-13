//----------------------------------------------------------------------------
//          File:           QueryGetEligibility.h
//          Description:    QueryGetEligibility
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
#include "DBAccess/EligibilityInfo.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetEligibility : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetEligibility(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetEligibility(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetEligibility() {};
  virtual const char* getQueryName() const override;

  void findEligibility(std::vector<const tse::EligibilityInfo*>& elges,
                       const VendorCode& vendor,
                       int itemNo);

  static void initialize();

  const QueryGetEligibility& operator=(const QueryGetEligibility& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetEligibility& operator=(const std::string& Another)
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
}; // class QueryGetEligibility

class QueryGetEligibilityHistorical : public QueryGetEligibility
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetEligibilityHistorical(DBAdapter* dbAdapt) : QueryGetEligibility(dbAdapt, _baseSQL) {}
  virtual ~QueryGetEligibilityHistorical() {}

  virtual const char* getQueryName() const override;

  void findEligibility(std::vector<const tse::EligibilityInfo*>& elges,
                       const VendorCode& vendor,
                       int itemNo,
                       const DateTime& startDate,
                       const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetEligibilityHistorical
} // namespace tse

