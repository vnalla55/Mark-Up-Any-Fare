//----------------------------------------------------------------------------
//          File:           QueryGetMarket.h
//          Description:    QueryGetMarket
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
#include "DBAccess/Loc.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{

class QueryGetMkt : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMkt(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetMkt(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetMkt() {};
  virtual const char* getQueryName() const override;

  void findLoc(std::vector<tse::Loc*>& locs, const LocCode& loc);

  static void initialize();

  const QueryGetMkt& operator=(const QueryGetMkt& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetMkt& operator=(const std::string& Another)
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
}; // class QueryGetMkt

class QueryGetMktHistorical : public QueryGetMkt
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMktHistorical(DBAdapter* dbAdapt) : QueryGetMkt(dbAdapt, _baseSQL) {};
  virtual ~QueryGetMktHistorical() {}
  virtual const char* getQueryName() const override;

  void findLoc(std::vector<tse::Loc*>& locs,
               const LocCode& loc,
               const DateTime& startDate,
               const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetMktHistorical

class QueryGetAllMkt : public QueryGetMkt
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllMkt(DBAdapter* dbAdapt) : QueryGetMkt(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllMkt() {}
  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::Loc*>& locs) { findAllLoc(locs); }

  void findAllLoc(std::vector<tse::Loc*>& locs);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllMkt
} // namespace tse

