//----------------------------------------------------------------------------
//          File:           QueryGetRuleCatDesc.h
//          Description:    QueryGetRuleCatDesc
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
#include "DBAccess/RuleCategoryDescInfo.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetRuleCatDesc : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetRuleCatDesc(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetRuleCatDesc(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetRuleCatDesc() {};

  virtual const char* getQueryName() const override;

  void findRuleCategoryDesc(tse::RuleCategoryDescInfo*& info, const CatNumber& catNumber);

  static void initialize();

  const QueryGetRuleCatDesc& operator=(const QueryGetRuleCatDesc& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetRuleCatDesc& operator=(const std::string& Another)
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
}; // class QueryGetRuleCatDesc

class QueryGetAllRuleCatDescs : public QueryGetRuleCatDesc
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllRuleCatDescs(DBAdapter* dbAdapt) : QueryGetRuleCatDesc(dbAdapt, _baseSQL) {};

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::RuleCategoryDescInfo*>& infos) { findAllRuleCategoryDesc(infos); }

  void findAllRuleCategoryDesc(std::vector<tse::RuleCategoryDescInfo*>& infos);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllRuleCatDescs
} // namespace tse

