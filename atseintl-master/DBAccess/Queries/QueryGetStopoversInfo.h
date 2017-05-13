//----------------------------------------------------------------------------
//          File:           QueryGetStopoversInfo.h
//          Description:    QueryGetStopoversInfo
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
#include "DBAccess/StopoversInfo.h"

namespace tse
{

class QueryGetStopoversInfo : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetStopoversInfo(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetStopoversInfo(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetStopoversInfo() {};
  virtual const char* getQueryName() const override;

  void findStopoversInfo(std::vector<tse::StopoversInfo*>& lstSI, VendorCode& vendor, int itemNo);

  static void initialize();

  const QueryGetStopoversInfo& operator=(const QueryGetStopoversInfo& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetStopoversInfo& operator=(const std::string& Another)
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
}; // class QueryGetStopoversInfo

class QueryGetStopoversInfoHistorical : public QueryGetStopoversInfo
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetStopoversInfoHistorical(DBAdapter* dbAdapt) : QueryGetStopoversInfo(dbAdapt, _baseSQL) {}
  virtual ~QueryGetStopoversInfoHistorical() {}
  virtual const char* getQueryName() const override;

  void findStopoversInfo(std::vector<tse::StopoversInfo*>& lstSI,
                         VendorCode& vendor,
                         int itemNo,
                         const DateTime& startDate,
                         const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetStopoversInfoHistorical
} // namespace tse
