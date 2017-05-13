//----------------------------------------------------------------------------
//          File:           QueryGetTaxRestrictionLocation.h
//          Description:    QueryGetTaxRestrictionLocation
//          Created:        1/14/2010
//          Authors:        Piotr Badarycz
//
//          Updates:
//
//     (c)2010, Sabre Inc. All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "Common/Logger.h"
#include "DBAccess/SQLQuery.h"
#include "DBAccess/TaxRestrictionLocationInfo.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetTaxRestrictionLocation : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxRestrictionLocation(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetTaxRestrictionLocation(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetTaxRestrictionLocation() {}

  virtual const char* getQueryName() const override;

  void findTaxRestrictionLocation(std::vector<const tse::TaxRestrictionLocationInfo*>& locations,
                                  TaxRestrictionLocation& location);

  static void initialize();
  const QueryGetTaxRestrictionLocation& operator=(const QueryGetTaxRestrictionLocation& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTaxRestrictionLocation& operator=(const std::string& Another)
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
}; // class QueryGetTaxRestrictionLocation

class QueryGetTaxRestrictionLocationHistorical : public QueryGetTaxRestrictionLocation
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxRestrictionLocationHistorical(DBAdapter* dbAdapt)
    : QueryGetTaxRestrictionLocation(dbAdapt, _baseSQL)
  {
  }
  QueryGetTaxRestrictionLocationHistorical(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : QueryGetTaxRestrictionLocation(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetTaxRestrictionLocationHistorical() {}

  virtual const char* getQueryName() const override;
  void findTaxRestrictionLocation(std::vector<const tse::TaxRestrictionLocationInfo*>& locations,
                                  TaxRestrictionLocation& location,
                                  const DateTime& startDate,
                                  const DateTime& endDate);
  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetTaxRestrictionLocationHistorical

} // namespace tse

