//----------------------------------------------------------------------------
//     � 2010, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "Common/Logger.h"
#include "DBAccess/BaggageSectorException.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetBaggageSectorException : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetBaggageSectorException(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetBaggageSectorException(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetBaggageSectorException() {};

  virtual const char* getQueryName() const override;

  void findBaggageSectorException(std::vector<tse::BaggageSectorException*>& exceptions,
                                  const CarrierCode& carrier);

  static void initialize();

  const QueryGetBaggageSectorException& operator=(const QueryGetBaggageSectorException& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetBaggageSectorException& operator=(const std::string& Another)
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
};
class QueryGetBaggageSectorExceptionHistorical : public tse::QueryGetBaggageSectorException
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetBaggageSectorExceptionHistorical(DBAdapter* dbAdapt)
    : QueryGetBaggageSectorException(dbAdapt, _baseSQL) {};
  virtual ~QueryGetBaggageSectorExceptionHistorical() {};
  virtual const char* getQueryName() const override;

  void findBaggageSectorException(std::vector<tse::BaggageSectorException*>& exceptions,
                                  const CarrierCode& carrier);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

class QueryGetAllBaggageSectorException : public QueryGetBaggageSectorException
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllBaggageSectorException(DBAdapter* dbAdapt)
    : QueryGetBaggageSectorException(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllBaggageSectorException() {};
  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::BaggageSectorException*>& exceptions)
  {
    findAllBaggageSectorException(exceptions);
  }

  void findAllBaggageSectorException(std::vector<tse::BaggageSectorException*>& exceptions);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

class QueryGetAllBaggageSectorExceptionHistorical : public QueryGetBaggageSectorException
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllBaggageSectorExceptionHistorical(DBAdapter* dbAdapt)
    : QueryGetBaggageSectorException(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllBaggageSectorExceptionHistorical() {};
  virtual const char* getQueryName() const override;

  void findAllBaggageSectorException(std::vector<tse::BaggageSectorException*>& exceptions);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
} // namespace tse

