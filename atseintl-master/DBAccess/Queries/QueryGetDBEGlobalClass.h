//----------------------------------------------------------------------------
//          File:           QueryGetDBEGlobalClass.h
//          Description:    QueryGetDBEGlobalClass
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
#include "DBAccess/DBEGlobalClass.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetDBEGlobalClass : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetDBEGlobalClass(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetDBEGlobalClass(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetDBEGlobalClass() {};

  virtual const char* getQueryName() const override;

  void findDBEGlobalClass(std::vector<tse::DBEGlobalClass*>& lstDBE, const DBEClass& dbeClass);

  static void initialize();

  const QueryGetDBEGlobalClass& operator=(const QueryGetDBEGlobalClass& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetDBEGlobalClass& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  };

protected:
private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetDBEGlobalClass

class QueryGetAllDBEGlobalClass : public QueryGetDBEGlobalClass
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllDBEGlobalClass(DBAdapter* dbAdapt) : QueryGetDBEGlobalClass(dbAdapt, _baseSQL) {};

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::DBEGlobalClass*>& lstDBE) { findAllDBEGlobalClass(lstDBE); }

  void findAllDBEGlobalClass(std::vector<tse::DBEGlobalClass*>& lstDBE);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllDBEGlobalClass
} // namespace tse

