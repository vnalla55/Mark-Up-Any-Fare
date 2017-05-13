//----------------------------------------------------------------------------
//          File:           QueryGetCabin.h
//          Description:    QueryGetCabin
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
#include "DBAccess/Cabin.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetCabin : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCabin(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetCabin(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetCabin() {};

  virtual const char* getQueryName() const override;

  void findCabin(std::vector<tse::Cabin*>& cabins, const CarrierCode& carrier);

  static void initialize();

  const QueryGetCabin& operator=(const QueryGetCabin& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetCabin& operator=(const std::string& Another)
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
}; // class QueryGetCabin

class QueryGetCabinHistorical : public tse::QueryGetCabin
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCabinHistorical(DBAdapter* dbAdapt) : QueryGetCabin(dbAdapt, _baseSQL) {};
  virtual ~QueryGetCabinHistorical() {};

  virtual const char* getQueryName() const override;

  void findCabin(std::vector<tse::Cabin*>& cabins, const CarrierCode& carrier);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetCabin

class QueryGetAllCabin : public QueryGetCabin
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllCabin(DBAdapter* dbAdapt) : QueryGetCabin(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllCabin() {};
  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::Cabin*>& cabins) { findAllCabin(cabins); }

  void findAllCabin(std::vector<tse::Cabin*>& cabins);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllCabin

class QueryGetAllCabinHistorical : public QueryGetCabin
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllCabinHistorical(DBAdapter* dbAdapt) : QueryGetCabin(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllCabinHistorical() {};
  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::Cabin*>& cabins) { findAllCabin(cabins); }

  void findAllCabin(std::vector<tse::Cabin*>& cabins);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllCabinHistorical

} // namespace tse

