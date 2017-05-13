//----------------------------------------------------------------------------
//          File:           QueryGetAccompaniedTravelInfo.h
//          Description:    QueryGetAccompaniedTravelInfo
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
#include "DBAccess/AccompaniedTravelInfo.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetAccompaniedTravelInfo : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAccompaniedTravelInfo(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetAccompaniedTravelInfo(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetAccompaniedTravelInfo() {};

  virtual const char* getQueryName() const override;

  void findAccompaniedTravelInfo(std::vector<tse::AccompaniedTravelInfo*>& lstATI,
                                 VendorCode& vendor,
                                 int itemNo);

  static void initialize();

  const QueryGetAccompaniedTravelInfo& operator=(const QueryGetAccompaniedTravelInfo& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetAccompaniedTravelInfo& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  };

protected:
private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAccompaniedTravelInfo

class QueryGetAccompaniedTravelInfoHistorical : public QueryGetAccompaniedTravelInfo
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAccompaniedTravelInfoHistorical(DBAdapter* dbAdapt)
    : QueryGetAccompaniedTravelInfo(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetAccompaniedTravelInfoHistorical() {}

  virtual const char* getQueryName() const override;

  void findAccompaniedTravelInfo(std::vector<tse::AccompaniedTravelInfo*>& lstATI,
                                 VendorCode& vendor,
                                 int itemNo,
                                 const DateTime& startDate,
                                 const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAccompaniedTravelInfoHistorical
} // namespace tse

