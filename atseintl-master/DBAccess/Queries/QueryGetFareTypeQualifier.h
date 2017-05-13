//----------------------------------------------------------------------------
//          File:           QueryGetFareTypeQualifier.h
//          Description:    QueryGetFareTypeQualifier
//          Created:        03/06/2007
//          Authors:        Quan Ta
//
//          Updates:
//
// � 2007, Sabre Inc.  All rights reserved.  This software/documentation is
// the confidential and proprietary product of Sabre Inc. Any unauthorized
// use, reproduction, or transfer of this software/documentation, in any
// medium, or incorporation of this software/documentation into any system or
// publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "Common/Logger.h"
#include "DBAccess/FareTypeQualifier.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetFareTypeQualPsg : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  friend class QueryGetFareTypeQualifier;

  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareTypeQualPsg(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetFareTypeQualPsg(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetFareTypeQualPsg() {};

  virtual const char* getQueryName() const override;

  void findFareTypeQualPsg(FareTypeQualifier& ftq);

  static void initialize();

  const QueryGetFareTypeQualPsg& operator=(const QueryGetFareTypeQualPsg& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetFareTypeQualPsg& operator=(const std::string& Another)
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

class QueryGetFareTypeQualifier : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize()
  {
    _isInitialized = false;
    QueryGetFareTypeQualPsg::deinitialize();
  }

public:
  QueryGetFareTypeQualifier(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetFareTypeQualifier(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetFareTypeQualifier() {};

  virtual const char* getQueryName() const override;

  void findFareTypeQualifier(std::vector<tse::FareTypeQualifier*>& ftQualifiers,
                             const Indicator userApplType,
                             const UserApplCode& userAppl,
                             const FareType& fareTypeQualifier);

  static void initialize();

  const QueryGetFareTypeQualifier& operator=(const QueryGetFareTypeQualifier& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetFareTypeQualifier& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  };

  void findFareTypeQualPsg(std::vector<FareTypeQualifier*>& ftqList);

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

class QueryGetAllFareTypeQualifier : public QueryGetFareTypeQualifier
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllFareTypeQualifier(DBAdapter* dbAdapt)
    : QueryGetFareTypeQualifier(dbAdapt, _baseSQL) {};

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::FareTypeQualifier*>& ftqList) { findAllFareTypeQualifier(ftqList); }

  void findAllFareTypeQualifier(std::vector<tse::FareTypeQualifier*>& ftqList);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

///////////////////////////////////////////////////////////
//
//  Historical
//
///////////////////////////////////////////////////////////

class QueryGetFareTypeQualPsgHistorical : public QueryGetFareTypeQualPsg
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareTypeQualPsgHistorical(DBAdapter* dbAdapt)
    : QueryGetFareTypeQualPsg(dbAdapt, _baseSQL) {};

  virtual const char* getQueryName() const override;

  void findFareTypeQualPsg(FareTypeQualifier& ftq);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

class QueryGetFareTypeQualifierHistorical : public QueryGetFareTypeQualifier
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareTypeQualifierHistorical(DBAdapter* dbAdapt)
    : QueryGetFareTypeQualifier(dbAdapt, _baseSQL) {};

  virtual const char* getQueryName() const override;

  void findFareTypeQualifier(std::vector<tse::FareTypeQualifier*>& ftQualifiers,
                             const Indicator userApplType,
                             const UserApplCode& userAppl,
                             const FareType& fareTypeQualifier);

  static void initialize();

  void findFareTypeQualPsg(std::vector<FareTypeQualifier*>& ftqList);

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

class QueryGetAllFareTypeQualifierHistorical : public QueryGetFareTypeQualifier
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllFareTypeQualifierHistorical(DBAdapter* dbAdapt)
    : QueryGetFareTypeQualifier(dbAdapt, _baseSQL) {};

  virtual const char* getQueryName() const override;

  void findAllFareTypeQualifier(std::vector<tse::FareTypeQualifier*>& ftqList);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

} // namespace tse

