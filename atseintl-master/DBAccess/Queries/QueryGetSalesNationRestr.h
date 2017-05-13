//----------------------------------------------------------------------------
//          File:           QueryGetSalesNationRestr.h
//          Description:    QueryGetSalesNationRestr
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
#include "DBAccess/SalesNationRestr.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetSalesNatRestrGovCxrs : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSalesNatRestrGovCxrs(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  virtual ~QueryGetSalesNatRestrGovCxrs() {};

  void resetSQL()
  {
    *this = _baseSQL;
  };

  const char* getQueryName() const override;

  void getGovCxrs(SalesNationRestr* a_pSalesNationRestr);

  static void initialize();

  const QueryGetSalesNatRestrGovCxrs& operator=(const QueryGetSalesNatRestrGovCxrs& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetSalesNatRestrGovCxrs& operator=(const std::string& Another)
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
}; // class QueryGetSalesNatRestrGovCxrs

class QueryGetSalesNatRestrTktgCxrs : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSalesNatRestrTktgCxrs(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  virtual ~QueryGetSalesNatRestrTktgCxrs() {};

  void resetSQL()
  {
    *this = _baseSQL;
  };

  const char* getQueryName() const override;

  void getTktgCxrs(SalesNationRestr* a_pSalesNationRestr);

  static void initialize();

  const QueryGetSalesNatRestrTktgCxrs& operator=(const QueryGetSalesNatRestrTktgCxrs& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetSalesNatRestrTktgCxrs& operator=(const std::string& Another)
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
}; // class QueryGetSalesNatRestrTktgCxrs

class QueryGetSalesNatRestrText : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSalesNatRestrText(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  virtual ~QueryGetSalesNatRestrText() {};

  void resetSQL()
  {
    *this = _baseSQL;
  };

  const char* getQueryName() const override;

  void getText(SalesNationRestr* a_pSalesNationRestr);

  static void initialize();

  const QueryGetSalesNatRestrText& operator=(const QueryGetSalesNatRestrText& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetSalesNatRestrText& operator=(const std::string& Another)
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
}; // class QueryGetSalesNatRestrText

class QueryGetSalesNationRestrBase : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSalesNationRestrBase(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetSalesNationRestrBase(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetSalesNationRestrBase() {};

  virtual const char* getQueryName() const override;

  void findSalesNationRestr(std::vector<tse::SalesNationRestr*>& lstSNR, NationCode& nation);

  static void initialize();

  const QueryGetSalesNationRestrBase& operator=(const QueryGetSalesNationRestrBase& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetSalesNationRestrBase& operator=(const std::string& Another)
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
}; // class QueryGetSalesNationRestrBase

class QueryGetSalesNationRestrBaseHistorical : public tse::QueryGetSalesNationRestrBase
{
public:
  QueryGetSalesNationRestrBaseHistorical(DBAdapter* dbAdapt)
    : QueryGetSalesNationRestrBase(dbAdapt, _baseSQL) {};
  virtual ~QueryGetSalesNationRestrBaseHistorical() {};
  virtual const char* getQueryName() const override;

  void findSalesNationRestr(std::vector<tse::SalesNationRestr*>& lstSNR, NationCode& nation);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetSalesNationRestrBaseHistorical

class QueryGetAllSalesNationRestrBaseHistorical : public tse::QueryGetSalesNationRestrBase
{
public:
  QueryGetAllSalesNationRestrBaseHistorical(DBAdapter* dbAdapt)
    : QueryGetSalesNationRestrBase(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllSalesNationRestrBaseHistorical() {};
  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::SalesNationRestr*>& lstSNR) { findAllSalesNationRestr(lstSNR); }

  void findAllSalesNationRestr(std::vector<tse::SalesNationRestr*>& lstSNR);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllSalesNationRestrBaseHistorical
} // namespace tse

