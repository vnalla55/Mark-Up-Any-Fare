//----------------------------------------------------------------------------
//          File:           QueryGetRuleCatAlphaCode.h
//          Description:    QueryGetRuleCatAlphaCode
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
#include "DBAccess/RuleCatAlphaCode.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetRuleCatAlphaCode : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetRuleCatAlphaCode(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetRuleCatAlphaCode(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetRuleCatAlphaCode() {};

  virtual const char* getQueryName() const override;

  void findRuleCatAlphaCode(std::vector<tse::RuleCatAlphaCode*>& infos, const AlphaCode& alphaCode);

  static void initialize();

  const QueryGetRuleCatAlphaCode& operator=(const QueryGetRuleCatAlphaCode& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetRuleCatAlphaCode& operator=(const std::string& Another)
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
}; // class QueryGetRuleCatAlphaCode

class QueryGetAllRuleCatAlphaCode : public QueryGetRuleCatAlphaCode
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllRuleCatAlphaCode(DBAdapter* dbAdapt) : QueryGetRuleCatAlphaCode(dbAdapt, _baseSQL) {};

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::RuleCatAlphaCode*>& infos) { findAllRuleCatAlphaCode(infos); }

  void findAllRuleCatAlphaCode(std::vector<tse::RuleCatAlphaCode*>& infos);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllRuleCatAlphaCode
} // namespace tse

