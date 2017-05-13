//----------------------------------------------------------------------------
//          File:           QueryGetBkgCodeConv.h
//          Description:    QueryGetBkgCodeConv
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
#include "DBAccess/BookingCodeConv.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{

class QueryGetBkgCodeConv : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetBkgCodeConv(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetBkgCodeConv(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetBkgCodeConv() {};

  virtual const char* getQueryName() const override;

  void findBookingCodeConv(std::vector<tse::BookingCodeConv*>& bkgCodeConvs,
                           const VendorCode& vendor,
                           const CarrierCode& carrier,
                           const TariffNumber& ruleTariff,
                           const RuleNumber& rule);

  static void initialize();

  const QueryGetBkgCodeConv& operator=(const QueryGetBkgCodeConv& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetBkgCodeConv& operator=(const std::string& Another)
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
}; // class QueryGetBkgCodeConv

class QueryGetAllBkgCodeConv : public QueryGetBkgCodeConv
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllBkgCodeConv(DBAdapter* dbAdapt) : QueryGetBkgCodeConv(dbAdapt, _baseSQL) {};
  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::BookingCodeConv*>& bkgCodeConvs)
  {
    findAllBookingCodeConv(bkgCodeConvs);
  }

  void findAllBookingCodeConv(std::vector<tse::BookingCodeConv*>& bkgCodeConvs);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllBkgCodeConv

class QueryGetBkgCodeConvHistorical : public tse::QueryGetBkgCodeConv
{
public:
  QueryGetBkgCodeConvHistorical(DBAdapter* dbAdapt) : QueryGetBkgCodeConv(dbAdapt, _baseSQL) {};
  virtual ~QueryGetBkgCodeConvHistorical() {};
  virtual const char* getQueryName() const override;

  void findBookingCodeConv(std::vector<tse::BookingCodeConv*>& bkgCodeConvs,
                           const VendorCode& vendor,
                           const CarrierCode& carrier,
                           const TariffNumber& ruleTariff,
                           const RuleNumber& rule,
                           const DateTime& startDate,
                           const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetBkgCodeConvHistorical
} // namespace tse
