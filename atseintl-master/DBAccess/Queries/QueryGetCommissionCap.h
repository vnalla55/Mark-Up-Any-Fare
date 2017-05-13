//----------------------------------------------------------------------------
//          File:           QueryGetCommissionCap.h
//          Description:    QueryGetCommissionCap
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
#include "DBAccess/CommissionCap.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetCommissionCap : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCommissionCap(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetCommissionCap(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetCommissionCap() {};

  virtual const char* getQueryName() const override;

  void findCommissionCap(std::vector<tse::CommissionCap*>& lstCC,
                         const CarrierCode& carrier,
                         const CurrencyCode& cur);

  static void initialize();

  const QueryGetCommissionCap& operator=(const QueryGetCommissionCap& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetCommissionCap& operator=(const std::string& Another)
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
}; // class QueryGetCommissionCap
class QueryGetCommissionCapHistorical : public tse::QueryGetCommissionCap
{
public:
  QueryGetCommissionCapHistorical(DBAdapter* dbAdapt) : QueryGetCommissionCap(dbAdapt, _baseSQL) {};
  virtual ~QueryGetCommissionCapHistorical() {};
  virtual const char* getQueryName() const override;

  void findCommissionCap(std::vector<tse::CommissionCap*>& lstCC,
                         const CarrierCode& carrier,
                         const CurrencyCode& cur);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetCommissionCapHistorical

class QueryGetAllCommissionCap : public QueryGetCommissionCap
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllCommissionCap(DBAdapter* dbAdapt) : QueryGetCommissionCap(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllCommissionCap() {};
  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::CommissionCap*>& lstCC) { findAllCommissionCap(lstCC); }

  void findAllCommissionCap(std::vector<tse::CommissionCap*>& lstCC);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllCommissionCap
class QueryGetAllCommissionCapHistorical : public QueryGetCommissionCap
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllCommissionCapHistorical(DBAdapter* dbAdapt)
    : QueryGetCommissionCap(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllCommissionCapHistorical() {};
  virtual const char* getQueryName() const override;

  void findAllCommissionCap(std::vector<tse::CommissionCap*>& lstCC);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllCommissionCapHistorical
} // namespace tse

