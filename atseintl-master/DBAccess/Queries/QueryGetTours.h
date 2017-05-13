//----------------------------------------------------------------------------
//          File:           QueryGetTours.h
//          Description:    QueryGetTours
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
#include "DBAccess/Tours.h"

namespace tse
{

class QueryGetTours : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTours(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetTours(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetTours() {}
  virtual const char* getQueryName() const override;

  void findTours(std::vector<tse::Tours*>& tours, const VendorCode& vendor, int itemNo);

  static void initialize();

  const QueryGetTours& operator=(const QueryGetTours& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTours& operator=(const std::string& Another)
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
}; // class QueryGetTours

class QueryGetToursHistorical : public QueryGetTours
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetToursHistorical(DBAdapter* dbAdapt) : QueryGetTours(dbAdapt, _baseSQL) {}
  virtual ~QueryGetToursHistorical() {}
  virtual const char* getQueryName() const override;

  void findTours(std::vector<tse::Tours*>& tours,
                 const VendorCode& vendor,
                 int itemNo,
                 const DateTime& startDate,
                 const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetToursHistorical
} // namespace tse

