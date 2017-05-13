//----------------------------------------------------------------------------
//          File:           QueryGetFDSSorting.h
//          Description:    QueryGetFDSSorting
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
#include "DBAccess/FDSSorting.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{

class Row;

class QueryGetFDSSorting : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFDSSorting(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetFDSSorting(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetFDSSorting() {};

  virtual const char* getQueryName() const override;

  void findFDSSorting(std::vector<tse::FDSSorting*>& infos,
                      const Indicator& userApplType,
                      const UserApplCode& userAppl,
                      const Indicator& pseudoCityType,
                      const PseudoCityCode& pseudoCity,
                      const TJRGroup& tjrGroup);

  static void initialize();

  const QueryGetFDSSorting& operator=(const QueryGetFDSSorting& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetFDSSorting& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  };

protected:
  FDSSorting* mapRowToFDSSorting(Row* row);

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetFDSSorting

class QueryGetAllFDSSorting : public QueryGetFDSSorting
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllFDSSorting(DBAdapter* dbAdapt) : QueryGetFDSSorting(dbAdapt, _baseSQL) {};

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::FDSSorting*>& infos) { findAllFDSSorting(infos); }

  void findAllFDSSorting(std::vector<tse::FDSSorting*>& infos);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllFDSSorting
} // namespace tse

