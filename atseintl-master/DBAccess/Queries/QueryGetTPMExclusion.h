//----------------------------------------------------------------------------
//          File:           QueryGetTPMExclusion.h
//          Description:    QueryGetTPMExclusion
//          Created:        08/10/2009
//          Authors:        Adam Szalajko
//
//          Updates:
//
//     � 2007, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "Common/Logger.h"
#include "DBAccess/SQLQuery.h"
#include "DBAccess/TPMExclusion.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetTPMExclusion : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTPMExclusion(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetTPMExclusion(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetTPMExclusion() {};

  virtual const char* getQueryName() const override;

  void findTPMExcl(std::vector<tse::TPMExclusion*>& tpms, const CarrierCode& carrier);

  static void initialize();

  const QueryGetTPMExclusion& operator=(const QueryGetTPMExclusion& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetTPMExclusion& operator=(const std::string& Another)
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
class QueryGetTPMExclusionHistorical : public tse::QueryGetTPMExclusion
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTPMExclusionHistorical(DBAdapter* dbAdapt) : QueryGetTPMExclusion(dbAdapt, _baseSQL) {};
  virtual ~QueryGetTPMExclusionHistorical() {};
  virtual const char* getQueryName() const override;

  void findTPMExcl(std::vector<tse::TPMExclusion*>& tpms, const CarrierCode& carrier);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

class QueryGetAllTPMExclusion : public QueryGetTPMExclusion
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllTPMExclusion(DBAdapter* dbAdapt) : QueryGetTPMExclusion(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllTPMExclusion() {};
  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::TPMExclusion*>& tpms) { findAllTPMExcl(tpms); }

  void findAllTPMExcl(std::vector<tse::TPMExclusion*>& tpms);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

class QueryGetAllTPMExclusionHistorical : public QueryGetTPMExclusion
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllTPMExclusionHistorical(DBAdapter* dbAdapt)
    : QueryGetTPMExclusion(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllTPMExclusionHistorical() {};
  virtual const char* getQueryName() const override;

  void findAllTPMExcl(std::vector<tse::TPMExclusion*>& tpms);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
} // namespace tse

