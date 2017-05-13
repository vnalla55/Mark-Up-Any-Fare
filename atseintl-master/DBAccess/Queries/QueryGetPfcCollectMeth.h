//----------------------------------------------------------------------------
//          File:           QueryGetPfcCollectMeth.h
//          Description:    QueryGetPfcCollectMeth
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
#include "DBAccess/PfcCollectMeth.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetPfcCollectMeth : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetPfcCollectMeth(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetPfcCollectMeth(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetPfcCollectMeth() {}

  virtual const char* getQueryName() const override;

  void findPfcCollectMeth(std::vector<tse::PfcCollectMeth*>& lstCM, const CarrierCode& carrier);

  static void initialize();
  const QueryGetPfcCollectMeth& operator=(const QueryGetPfcCollectMeth& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetPfcCollectMeth& operator=(const std::string& Another)
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
}; // class QueryGetPfcCollectMeth

class QueryGetAllPfcCollectMeth : public QueryGetPfcCollectMeth
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllPfcCollectMeth(DBAdapter* dbAdapt) : QueryGetPfcCollectMeth(dbAdapt, _baseSQL) {}

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::PfcCollectMeth*>& lstCM) { findAllPfcCollectMeth(lstCM); }

  void findAllPfcCollectMeth(std::vector<tse::PfcCollectMeth*>& lstCM);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllPfcCollectMeth

class QueryGetPfcCollectMethHistorical : public QueryGetPfcCollectMeth
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetPfcCollectMethHistorical(DBAdapter* dbAdapt) : QueryGetPfcCollectMeth(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetPfcCollectMethHistorical() {}

  virtual const char* getQueryName() const override;

  void findPfcCollectMeth(std::vector<tse::PfcCollectMeth*>& lstCM,
                          const CarrierCode& carrier,
                          const DateTime& startDate,
                          const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetPfcCollectMethHistorical

} // namespace tse

