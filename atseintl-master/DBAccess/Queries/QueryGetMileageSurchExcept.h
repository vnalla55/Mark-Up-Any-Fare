//----------------------------------------------------------------------------
//          File:           QueryGetMileageSurchExcept.h
//          Description:    QueryGetMileageSurchExcept
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
#include "DBAccess/MileageSurchExcept.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{

class QueryGetMlgSurchPsgTypes : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMlgSurchPsgTypes(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  virtual ~QueryGetMlgSurchPsgTypes() {};
  void resetSQL()
  {
    *this = _baseSQL;
  };
  const char* getQueryName() const override;
  void getPsgTypes(MileageSurchExcept* a_pMileageSurchExcept);
  static void initialize();

  const QueryGetMlgSurchPsgTypes& operator=(const QueryGetMlgSurchPsgTypes& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetMlgSurchPsgTypes& operator=(const std::string& Another)
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
}; // class QueryGetMlgSurchPsgTypes

class QueryGetOneMileageSurchExceptBase : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetOneMileageSurchExceptBase(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetOneMileageSurchExceptBase(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetOneMileageSurchExceptBase() {};
  virtual const char* getQueryName() const override;

  void findMileageSurchExcept(std::vector<tse::MileageSurchExcept*>& lstMSE,
                              VendorCode& vendor,
                              int textTblItemNo,
                              CarrierCode& governingCarrier,
                              TariffNumber ruleTariff,
                              RuleNumber& rule);

  static void initialize();

  const QueryGetOneMileageSurchExceptBase&
  operator=(const QueryGetOneMileageSurchExceptBase& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetOneMileageSurchExceptBase& operator=(const std::string& Another)
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
}; // class QueryGetOneMileageSurchExceptBase

class QueryGetOneMileageSurchExceptBaseHistorical : public QueryGetOneMileageSurchExceptBase
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetOneMileageSurchExceptBaseHistorical(DBAdapter* dbAdapt)
    : QueryGetOneMileageSurchExceptBase(dbAdapt, _baseSQL) {};
  virtual ~QueryGetOneMileageSurchExceptBaseHistorical() {};
  virtual const char* getQueryName() const override;

  void findMileageSurchExcept(std::vector<tse::MileageSurchExcept*>& lstMSE,
                              VendorCode& vendor,
                              int textTblItemNo,
                              CarrierCode& governingCarrier,
                              TariffNumber ruleTariff,
                              RuleNumber& rule,
                              const DateTime& startDate,
                              const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetOneMileageSurchExceptBaseHistorical

class QueryGetAllMileageSurchExceptBase : public QueryGetOneMileageSurchExceptBase
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllMileageSurchExceptBase(DBAdapter* dbAdapt)
    : QueryGetOneMileageSurchExceptBase(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllMileageSurchExceptBase() {};
  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::MileageSurchExcept*>& lstMSE) { findAllMileageSurchExcept(lstMSE); }

  void findAllMileageSurchExcept(std::vector<tse::MileageSurchExcept*>& lstMSE);
  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllMileageSurchExceptBase

class QueryCxrInMlgSurchExcpt : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryCxrInMlgSurchExcpt(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryCxrInMlgSurchExcpt(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryCxrInMlgSurchExcpt() {};
  virtual const char* getQueryName() const override;
  static void initialize();
  const QueryCxrInMlgSurchExcpt& operator=(const QueryCxrInMlgSurchExcpt& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryCxrInMlgSurchExcpt& operator=(const std::string& Another)
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
}; // class QueryCxrInMlgSurchExcpt

class QueryCxrInMlgSurchExcptHistorical : public tse::QueryCxrInMlgSurchExcpt
{
public:
  QueryCxrInMlgSurchExcptHistorical(DBAdapter* dbAdapt)
    : QueryCxrInMlgSurchExcpt(dbAdapt, _baseSQL) {};
  virtual ~QueryCxrInMlgSurchExcptHistorical() {};
  virtual const char* getQueryName() const override;

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryCxrInMlgSurchExcptHistorical
} // namespace tse

