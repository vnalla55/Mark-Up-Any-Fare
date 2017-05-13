//----------------------------------------------------------------------------
//          File:           QueryGetPfcTktDesigExcept.h
//          Description:    QueryGetPfcTktDesigExcept
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
#include "DBAccess/PfcTktDesigExcept.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetPfcTktDesigExcept : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetPfcTktDesigExcept(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetPfcTktDesigExcept(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetPfcTktDesigExcept() {}

  virtual const char* getQueryName() const override;

  void findPfcTktDesigExcept(std::vector<const tse::PfcTktDesigExcept*>& tde,
                             const CarrierCode& carrier);

  static void initialize();
  const QueryGetPfcTktDesigExcept& operator=(const QueryGetPfcTktDesigExcept& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetPfcTktDesigExcept& operator=(const std::string& Another)
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
}; // class QueryGetPfcTktDesigExcept

class QueryGetPfcTktDesigExceptHistorical : public QueryGetPfcTktDesigExcept
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetPfcTktDesigExceptHistorical(DBAdapter* dbAdapt)
    : QueryGetPfcTktDesigExcept(dbAdapt, _baseSQL)
  {
  }
  QueryGetPfcTktDesigExceptHistorical(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : QueryGetPfcTktDesigExcept(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetPfcTktDesigExceptHistorical() {}

  virtual const char* getQueryName() const override;

  void findPfcTktDesigExcept(std::vector<const tse::PfcTktDesigExcept*>& tde,
                             const CarrierCode& carrier,
                             const DateTime& startDate,
                             const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetPfcTktDesigExcept
} // namespace tse

