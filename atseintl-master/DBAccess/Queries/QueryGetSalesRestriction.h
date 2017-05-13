//----------------------------------------------------------------------------
//          File:           QueryGetSalesRestriction.h
//          Description:    QueryGetSalesRestriction
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
#include "DBAccess/SalesRestriction.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{

class QueryGetSalesRestriction : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSalesRestriction(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetSalesRestriction(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetSalesRestriction() {};
  virtual const char* getQueryName() const override;

  void
  findSalesRestriction(std::vector<tse::SalesRestriction*>& lstSR, VendorCode& vendor, int itemNo);

  static void initialize();

  const QueryGetSalesRestriction& operator=(const QueryGetSalesRestriction& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetSalesRestriction& operator=(const std::string& Another)
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
}; // QueryGetSalesRestriction

class QueryGetSalesRestrictionHistorical : public QueryGetSalesRestriction
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSalesRestrictionHistorical(DBAdapter* dbAdapt)
    : QueryGetSalesRestriction(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetSalesRestrictionHistorical() {}
  virtual const char* getQueryName() const override;

  void findSalesRestriction(std::vector<tse::SalesRestriction*>& lstSR,
                            VendorCode& vendor,
                            int itemNo,
                            const DateTime& startDate,
                            const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetSalesRestrictionHistorical
} // namespace tse
