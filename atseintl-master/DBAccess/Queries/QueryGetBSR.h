//----------------------------------------------------------------------------
//          File:           QueryGetBSR.h
//          Description:    QueryGetBSR
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
#include "DBAccess/BankerSellRate.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{

class QueryGetBSR : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetBSR(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetBSR(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetBSR() {};
  virtual const char* getQueryName() const override;

  void findBSR(std::vector<tse::BankerSellRate*>& BSRs, const CurrencyCode& primeCurrCode);

  static void initialize();

  const QueryGetBSR& operator=(const QueryGetBSR& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetBSR& operator=(const std::string& Another)
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
}; // class QueryGetBSR

class QueryGetBSRHistorical : public tse::QueryGetBSR
{
public:
  QueryGetBSRHistorical(DBAdapter* dbAdapt) : QueryGetBSR(dbAdapt, _baseSQL) {};
  virtual ~QueryGetBSRHistorical() {};
  virtual const char* getQueryName() const override;

  void findBSR(std::vector<tse::BankerSellRate*>& BSRs,
               const CurrencyCode& primeCurrCode,
               const DateTime& startDate,
               const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;

  void setAdapterAndBaseSQL(DBAdapter* dbAdapt);
}; // class QueryGetBSRHistorical

} // namespace tse

