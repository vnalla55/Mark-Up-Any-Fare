//----------------------------------------------------------------------------
//          File:           QueryGetNuc.h
//          Description:    QueryGetNuc
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
#include "DBAccess/NUCInfo.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetOneNuc : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetOneNuc(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetOneNuc(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetOneNuc() {};

  virtual const char* getQueryName() const override;

  void findNUC(std::vector<tse::NUCInfo*>& nucs, CurrencyCode cur, CarrierCode carrier);

  static void initialize();

  const QueryGetOneNuc& operator=(const QueryGetOneNuc& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetOneNuc& operator=(const std::string& Another)
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
}; // class QueryGetOneNuc

class QueryGetAllNucs : public QueryGetOneNuc
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllNucs(DBAdapter* dbAdapt) : QueryGetOneNuc(dbAdapt, _baseSQL) {};

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::NUCInfo*>& nucs) { findAllNUC(nucs); }

  void findAllNUC(std::vector<tse::NUCInfo*>& nucs);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllNucs

class QueryGetNucHistorical : public QueryGetOneNuc
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetNucHistorical(DBAdapter* dbAdapt) : QueryGetOneNuc(dbAdapt, _baseSQL) {};
  virtual ~QueryGetNucHistorical() {};

  virtual const char* getQueryName() const override;

  void findNUC(std::vector<tse::NUCInfo*>& nucs,
               CurrencyCode cur,
               CarrierCode carrier,
               const DateTime& startDate,
               const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetNucHistorical
} // namespace tse

