//----------------------------------------------------------------------------
//          File:           QueryGetTable993.h
//          Description:    QueryGetTable993
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
#include "DBAccess/SamePoint.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{

class QueryGetTable993 : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTable993(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetTable993(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetTable993() {};
  virtual const char* getQueryName() const override;

  void findSamePoint(std::vector<const tse::SamePoint*>& table993s,
                     const VendorCode& vendor,
                     int itemNo);

  static void initialize();

  const QueryGetTable993& operator=(const QueryGetTable993& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetTable993& operator=(const std::string& Another)
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
}; // class QueryGetTable993

class QueryGetTable993Historical : public QueryGetTable993
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTable993Historical(DBAdapter* dbAdapt) : QueryGetTable993(dbAdapt, _baseSQL) {}
  virtual ~QueryGetTable993Historical() {}
  virtual const char* getQueryName() const override;

  void findSamePoint(std::vector<const tse::SamePoint*>& table993s,
                     const VendorCode& vendor,
                     int itemNo,
                     const DateTime& startDate,
                     const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetTable993Historical
} // namespace tse
