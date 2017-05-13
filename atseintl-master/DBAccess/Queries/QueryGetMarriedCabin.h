//----------------------------------------------------------------------------
//          File:           QueryGetMarriedCabin.h
//          Description:    QueryGetMarriedCabin
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
#include "DBAccess/MarriedCabin.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetMarriedCabin : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMarriedCabin(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetMarriedCabin(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetMarriedCabin() {};

  virtual const char* getQueryName() const override;

  void findMarriedCabins(std::vector<MarriedCabin*>& mcList,
                         const CarrierCode& carrier,
                         const BookingCode& premiumCabin);

  static void initialize();

  const QueryGetMarriedCabin& operator=(const QueryGetMarriedCabin& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetMarriedCabin& operator=(const std::string& Another)
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
}; // class QueryGetMarriedCabin

class QueryGetAllMarriedCabinHistorical : public QueryGetMarriedCabin
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllMarriedCabinHistorical(DBAdapter* dbAdapt)
    : QueryGetMarriedCabin(dbAdapt, _baseSQL) {};

  virtual const char* getQueryName() const override;

  void execute(std::vector<MarriedCabin*>& mcList) { findAllMarriedCabins(mcList); }

  void findAllMarriedCabins(std::vector<MarriedCabin*>& mcList);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllMarriedCabinHistorical

class QueryGetMarriedCabinHistorical : public QueryGetMarriedCabin
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMarriedCabinHistorical(DBAdapter* dbAdapt) : QueryGetMarriedCabin(dbAdapt, _baseSQL) {};

  virtual const char* getQueryName() const override;

  void findMarriedCabins(std::vector<MarriedCabin*>& mcList,
                         const CarrierCode& carrier,
                         const BookingCode& premiumCabin);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetMarriedCabinHistorical

} // namespace tse

