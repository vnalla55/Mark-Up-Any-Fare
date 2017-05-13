//----------------------------------------------------------------------------
//          File:           QueryGetNoPNRFareTypeGroup.h
//          Description:    QueryGetNoPNRFareTypeGroup
//          Created:        1/11/2008
// Authors:         Karolina Golebiewska
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
#include "DBAccess/NoPNRFareTypeGroup.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetNoPNRFareTypeGroup : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetNoPNRFareTypeGroup(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetNoPNRFareTypeGroup(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetNoPNRFareTypeGroup() {};

  virtual const char* getQueryName() const override;

  void
  findNoPNRFareTypeGroup(std::vector<tse::NoPNRFareTypeGroup*>& npftgs, const int fareTypeGroup);

  static void initialize();

  const QueryGetNoPNRFareTypeGroup& operator=(const QueryGetNoPNRFareTypeGroup& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetNoPNRFareTypeGroup& operator=(const std::string& Another)
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
}; // class QueryGetNoPNRFareTypeGroup

class QueryGetAllNoPNRFareTypeGroup : public QueryGetNoPNRFareTypeGroup
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllNoPNRFareTypeGroup(DBAdapter* dbAdapt)
    : QueryGetNoPNRFareTypeGroup(dbAdapt, _baseSQL) {};

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::NoPNRFareTypeGroup*>& npftgs) { findAllNoPNRFareTypeGroup(npftgs); }

  void findAllNoPNRFareTypeGroup(std::vector<tse::NoPNRFareTypeGroup*>& npftgs);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllNoPNRFareTypeGroup
} // namespace tse

