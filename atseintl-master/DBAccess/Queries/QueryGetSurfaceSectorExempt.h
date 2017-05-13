//----------------------------------------------------------------------------
//          File:           QueryGetSurfaceSectorExempt.h
//          Description:    QueryGetSurfaceSectorExempt
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
#include "DBAccess/SQLQuery.h"
#include "DBAccess/SurfaceSectorExempt.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetSurfaceSectorExempt : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSurfaceSectorExempt(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetSurfaceSectorExempt(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetSurfaceSectorExempt() {};

  virtual const char* getQueryName() const override;

  void findSurfaceSectorExempt(std::vector<tse::SurfaceSectorExempt*>& lstSSE,
                               const LocCode& origLoc,
                               const LocCode& destLoc);

  static void initialize();

  const QueryGetSurfaceSectorExempt& operator=(const QueryGetSurfaceSectorExempt& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetSurfaceSectorExempt& operator=(const std::string& Another)
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
}; // class QueryGetSurfaceSectorExempt

class QueryGetSurfaceSectorExemptHistorical : public tse::QueryGetSurfaceSectorExempt
{
public:
  QueryGetSurfaceSectorExemptHistorical(DBAdapter* dbAdapt)
    : QueryGetSurfaceSectorExempt(dbAdapt, _baseSQL) {};
  virtual ~QueryGetSurfaceSectorExemptHistorical() {};

  virtual const char* getQueryName() const override;

  void findSurfaceSectorExempt(std::vector<tse::SurfaceSectorExempt*>& lstSSE,
                               const LocCode& origLoc,
                               const LocCode& destLoc);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetSurfaceSectorExemptHistorical

class QueryGetAllSurfaceSectorExempt : public QueryGetSurfaceSectorExempt
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllSurfaceSectorExempt(DBAdapter* dbAdapt)
    : QueryGetSurfaceSectorExempt(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllSurfaceSectorExempt() {};
  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::SurfaceSectorExempt*>& lstSSE)
  {
    findAllSurfaceSectorExempt(lstSSE);
  }

  void findAllSurfaceSectorExempt(std::vector<tse::SurfaceSectorExempt*>& lstSSE);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllSurfaceSectorExempt

class QueryGetAllSurfaceSectorExemptHistorical : public QueryGetSurfaceSectorExempt
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllSurfaceSectorExemptHistorical(DBAdapter* dbAdapt)
    : QueryGetSurfaceSectorExempt(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllSurfaceSectorExemptHistorical() {};
  virtual const char* getQueryName() const override;

  void findAllSurfaceSectorExempt(std::vector<tse::SurfaceSectorExempt*>& lstSSE);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllSurfaceSectorExemptHistorical
} // namespace tse

