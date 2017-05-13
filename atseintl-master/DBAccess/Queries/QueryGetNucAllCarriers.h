//----------------------------------------------------------------------------
//          File:           QueryGetNucAllCarriers.h
//          Description:    QueryGetNucAllCarriers
//          Created:        11/08/2007
//          Authors:        Tomasz Karczewski
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
#include "DBAccess/NUCInfo.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetOneNucAllCarriers : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetOneNucAllCarriers(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetOneNucAllCarriers(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetOneNucAllCarriers() {};

  virtual const char* getQueryName() const override;

  void findNUC(std::vector<tse::NUCInfo*>& nucs, CurrencyCode cur);

  static void initialize();

  const QueryGetOneNucAllCarriers& operator=(const QueryGetOneNucAllCarriers& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetOneNucAllCarriers& operator=(const std::string& Another)
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
}; // class QueryGetOneNucAllCarriers

class QueryGetAllNucsAllCarriers : public QueryGetOneNucAllCarriers
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllNucsAllCarriers(DBAdapter* dbAdapt) : QueryGetOneNucAllCarriers(dbAdapt, _baseSQL) {};

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::NUCInfo*>& nucs) { findAllNUC(nucs); }

  void findAllNUC(std::vector<tse::NUCInfo*>& nucs);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllNucsAllCarriers

class QueryGetNucAllCarriersHistorical : public QueryGetOneNucAllCarriers
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetNucAllCarriersHistorical(DBAdapter* dbAdapt)
    : QueryGetOneNucAllCarriers(dbAdapt, _baseSQL) {};
  virtual ~QueryGetNucAllCarriersHistorical() {};

  virtual const char* getQueryName() const override;

  void findNUC(std::vector<tse::NUCInfo*>& nucs, CurrencyCode cur);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;

}; // class QueryGetNucAllCarriersHistorical

} // namespace tse

