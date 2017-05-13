//----------------------------------------------------------------------------
//          File:           QueryGetCarrierMixedClass.h
//          Description:    QueryGetCarrierMixedClass
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
#include "DBAccess/CarrierMixedClass.h"
#include "DBAccess/CarrierMixedClassSeg.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetCarrierMixedClass : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCarrierMixedClass(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetCarrierMixedClass(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetCarrierMixedClass() {};

  virtual const char* getQueryName() const override;

  void findCarrierMixedClass(std::vector<tse::CarrierMixedClass*>& mixedClasses,
                             const CarrierCode& carrier);

  static void initialize();

  const QueryGetCarrierMixedClass& operator=(const QueryGetCarrierMixedClass& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetCarrierMixedClass& operator=(const std::string& Another)
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
}; // class QueryGetCarrierMixedClass

class QueryGetCarrierMixedClassHistorical : public tse::QueryGetCarrierMixedClass
{
public:
  QueryGetCarrierMixedClassHistorical(DBAdapter* dbAdapt)
    : QueryGetCarrierMixedClass(dbAdapt, _baseSQL) {};
  virtual ~QueryGetCarrierMixedClassHistorical() {};

  virtual const char* getQueryName() const override;

  void findCarrierMixedClass(std::vector<tse::CarrierMixedClass*>& mixedClasses,
                             const CarrierCode& carrier);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetCarrierMixedClassHistorical

class QueryGetAllCarrierMixedClass : public QueryGetCarrierMixedClass
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllCarrierMixedClass(DBAdapter* dbAdapt)
    : QueryGetCarrierMixedClass(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllCarrierMixedClass() {};
  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::CarrierMixedClass*>& mixedClasses)
  {
    findAllCarrierMixedClass(mixedClasses);
  }

  void findAllCarrierMixedClass(std::vector<tse::CarrierMixedClass*>& mixedClasses);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllCarrierMixedClass

class QueryGetAllCarrierMixedClassHistorical : public QueryGetCarrierMixedClass
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllCarrierMixedClassHistorical(DBAdapter* dbAdapt)
    : QueryGetCarrierMixedClass(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllCarrierMixedClassHistorical() {};
  virtual const char* getQueryName() const override;

  void findAllCarrierMixedClass(std::vector<tse::CarrierMixedClass*>& mixedClasses);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllCarrierMixedClassHistorical

} // namespace tse

