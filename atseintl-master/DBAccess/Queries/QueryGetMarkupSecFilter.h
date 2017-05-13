//----------------------------------------------------------------------------
//          File:           QueryGetMarkupSecFilter.h
//          Description:    QueryGetMarkupSecFilter
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
#include "DBAccess/MarkupSecFilter.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetMarkupSecFilter : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMarkupSecFilter(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetMarkupSecFilter(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetMarkupSecFilter() {};
  virtual const char* getQueryName() const override;

  void findMarkupSecFilter(std::vector<tse::MarkupSecFilter*>& lstMSF,
                           VendorCode& vendor,
                           CarrierCode& carrier,
                           TariffNumber ruleTariff,
                           RuleNumber& rule);

  static void initialize();

  const QueryGetMarkupSecFilter& operator=(const QueryGetMarkupSecFilter& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetMarkupSecFilter& operator=(const std::string& Another)
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
}; // class QueryGetMarkupSecFilter

class QueryGetMarkupSecFilterHistorical : public QueryGetMarkupSecFilter
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMarkupSecFilterHistorical(DBAdapter* dbAdapt) : QueryGetMarkupSecFilter(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetMarkupSecFilterHistorical() {}
  virtual const char* getQueryName() const override;

  void findMarkupSecFilter(std::vector<tse::MarkupSecFilter*>& lstMSF,
                           VendorCode& vendor,
                           CarrierCode& carrier,
                           TariffNumber ruleTariff,
                           RuleNumber& rule,
                           const DateTime& startDate,
                           const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;

  //    void setAdapterAndBaseSQL(DBAdapter * dbAdapt);

}; // class QueryGetMarkupSecFilterHistorical
} // namespace tse

