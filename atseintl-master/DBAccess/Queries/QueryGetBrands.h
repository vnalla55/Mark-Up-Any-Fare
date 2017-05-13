//----------------------------------------------------------------------------
//          File:           QueryGetBrands.h
//          Description:    QueryGetBrands
//          Created:        1/10/2007
//          Authors:        Mike Lillis / Marco Cartolano
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
#include "DBAccess/Brand.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetBrands : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetBrands(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetBrands(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetBrands() {};

  virtual const char* getQueryName() const override;

  void findBrands(std::vector<tse::Brand*>& infos,
                  const Indicator& userApplType,
                  const UserApplCode& userAppl,
                  const CarrierCode& carrier);

  static void initialize();

  const QueryGetBrands& operator=(const QueryGetBrands& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetBrands& operator=(const std::string& Another)
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
}; // class QueryGetBrands

class QueryGetBrandsHistorical : public tse::QueryGetBrands
{
public:
  QueryGetBrandsHistorical(DBAdapter* dbAdapt) : QueryGetBrands(dbAdapt, _baseSQL) {};
  virtual ~QueryGetBrandsHistorical() {};
  virtual const char* getQueryName() const override;

  void findBrands(std::vector<tse::Brand*>& infos,
                  const Indicator& userApplType,
                  const UserApplCode& userAppl,
                  const CarrierCode& carrier);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetBrandsHistorical

class QueryGetAllBrands : public QueryGetBrands
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllBrands(DBAdapter* dbAdapt) : QueryGetBrands(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllBrands() {};
  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::Brand*>& infos) { findAllBrands(infos); }

  void findAllBrands(std::vector<tse::Brand*>& infos);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllBrands

class QueryGetAllBrandsHistorical : public QueryGetBrands
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllBrandsHistorical(DBAdapter* dbAdapt) : QueryGetBrands(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllBrandsHistorical() {};
  virtual const char* getQueryName() const override;

  void findAllBrands(std::vector<tse::Brand*>& infos);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllBrands
} // namespace tse

