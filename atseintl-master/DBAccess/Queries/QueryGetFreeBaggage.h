//----------------------------------------------------------------------------
//          File:           QueryGetFreeBaggage.h
//          Description:    QueryGetFreeBaggage
//          Created:        06/27/2007
// Authors:         Slawomir Machowicz
//
//          Updates:
//
//     � 2007, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "DBAccess/FreeBaggageInfo.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{

class QueryGetFreeBaggage : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFreeBaggage(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetFreeBaggage(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetFreeBaggage() {};
  virtual const char* getQueryName() const override;

  void findFreeBaggage(std::vector<tse::FreeBaggageInfo*>& lstBag, CarrierCode carrier);

  static void initialize();

  const QueryGetFreeBaggage& operator=(const QueryGetFreeBaggage& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }

  const QueryGetFreeBaggage& operator=(const std::string& Another)
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
}; // class QueryGetFreeBaggage

class QueryGetFreeBaggageHistorical : public QueryGetFreeBaggage
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFreeBaggageHistorical(DBAdapter* dbAdapt) : QueryGetFreeBaggage(dbAdapt, _baseSQL) {};
  virtual ~QueryGetFreeBaggageHistorical() {}
  virtual const char* getQueryName() const override;

  void findFreeBaggage(std::vector<tse::FreeBaggageInfo*>& lstBag,
                       CarrierCode carrier,
                       const DateTime& startDate,
                       const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetFreeBaggageHistorical
}
