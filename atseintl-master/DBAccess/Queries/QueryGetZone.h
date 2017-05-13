//----------------------------------------------------------------------------
//          File:           QueryGetZone.h
//          Description:    QueryGetZone
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
#include "DBAccess/SQLQuery.h"
#include "DBAccess/ZoneInfo.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetZone : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetZone(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetZone(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetZone() {}

  virtual const char* getQueryName() const override;

  void findZone(std::vector<const tse::ZoneInfo*>& zones,
                VendorCode& vendor,
                Zone& zone,
                Indicator zoneType);

  static void initialize();
  const QueryGetZone& operator=(const QueryGetZone& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetZone& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  }

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetZone

class QueryGetZoneHistorical : public QueryGetZone
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetZoneHistorical(DBAdapter* dbAdapt) : QueryGetZone(dbAdapt, _baseSQL) {}
  QueryGetZoneHistorical(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : QueryGetZone(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetZoneHistorical() {}

  virtual const char* getQueryName() const override;
  void findZone(std::vector<const tse::ZoneInfo*>& zones,
                VendorCode& vendor,
                Zone& zone,
                Indicator zoneType,
                const DateTime& startDate,
                const DateTime& endDate);
  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetZoneHistorical

} // namespace tse

