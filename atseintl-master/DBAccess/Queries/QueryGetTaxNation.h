//----------------------------------------------------------------------------
//          File:           QueryGetTaxNation.h
//          Description:    QueryGetTaxNation
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
#include "DBAccess/TaxNation.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetTaxNRound : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxNRound(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  virtual ~QueryGetTaxNRound() {}
  void resetSQL()
  {
    *this = _baseSQL;
  };
  const char* getQueryName() const override;
  void getExceptNation(tse::TaxNation* a_pTaxNation, const NationCode& nationCode);
  static void initialize();
  const QueryGetTaxNRound& operator=(const QueryGetTaxNRound& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTaxNRound& operator=(const std::string& Another)
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
}; // class QueryGetTaxNRound

class QueryGetTaxNCollect : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxNCollect(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  virtual ~QueryGetTaxNCollect() {}
  void resetSQL()
  {
    *this = _baseSQL;
  };
  const char* getQueryName() const override;
  void getCollectNation(tse::TaxNation* a_pTaxNation, const NationCode& nationCode);
  static void initialize();
  const QueryGetTaxNCollect& operator=(const QueryGetTaxNCollect& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTaxNCollect& operator=(const std::string& Another)
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
}; // class QueryGetTaxNCollect

class QueryGetTaxOrderTktIssue : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxOrderTktIssue(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  virtual ~QueryGetTaxOrderTktIssue() {}
  void resetSQL()
  {
    *this = _baseSQL;
  };
  const char* getQueryName() const override;
  void getOrderTktIssue(tse::TaxNation* a_pTaxNation, const NationCode& nationCode);
  static void initialize();
  const QueryGetTaxOrderTktIssue& operator=(const QueryGetTaxOrderTktIssue& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTaxOrderTktIssue& operator=(const std::string& Another)
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
}; // class QueryGetTaxOrderTktIssue

class QueryGetTaxNation : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxNation(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetTaxNation(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetTaxNation() {}

  const char* getQueryName() const override;

  void findTaxNation(std::vector<tse::TaxNation*>& TaxN, const NationCode& nationCode);

  static void initialize();
  const QueryGetTaxNation& operator=(const QueryGetTaxNation& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTaxNation& operator=(const std::string& Another)
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
}; // class QueryGetTaxNation

class QueryGetTaxNationHistorical : public QueryGetTaxNation
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxNationHistorical(DBAdapter* dbAdapt) : QueryGetTaxNation(dbAdapt, _baseSQL) {}
  virtual ~QueryGetTaxNationHistorical() {}

  const char* getQueryName() const override;

  void findTaxNation(std::vector<tse::TaxNation*>& TaxN,
                     const NationCode& nationCode,
                     const DateTime& startDate,
                     const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetTaxNationHistorical

} // namespace tse

