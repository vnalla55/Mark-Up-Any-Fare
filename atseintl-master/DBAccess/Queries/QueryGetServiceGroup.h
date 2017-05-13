//----------------------------------------------------------------------------
//          File:           QueryGetAllServiceGroup.h
//          Description:    QueryGetAllServiceGroup.h
//          Created:        08/2/2010
// Authors:
//
//          Updates:
//
//      2010, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "DBAccess/ServiceGroupInfo.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{

class QueryGetAllServiceGroup : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllServiceGroup(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetAllServiceGroup(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetAllServiceGroup() {};
  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::ServiceGroupInfo*>& groups) { findAllServiceGroup(groups); }

  void findAllServiceGroup(std::vector<tse::ServiceGroupInfo*>& groups);

  static void initialize();

  const QueryGetAllServiceGroup& operator=(const QueryGetAllServiceGroup& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetAllServiceGroup& operator=(const std::string& Another)
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
}; // class QueryGetAllServiceGroup

class QueryGetAllServiceGroupHistorical : public QueryGetAllServiceGroup
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllServiceGroupHistorical(DBAdapter* dbAdapt)
    : QueryGetAllServiceGroup(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllServiceGroupHistorical() {}
  virtual const char* getQueryName() const override;

  void findAllServiceGroup(std::vector<tse::ServiceGroupInfo*>& groups);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllServiceGroupHistorical

} // namespace tse

