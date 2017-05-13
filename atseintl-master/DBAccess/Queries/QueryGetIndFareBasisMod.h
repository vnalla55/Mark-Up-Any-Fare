//----------------------------------------------------------------------------
//          File:           QueryGetIndFareBasisMod.h
//          Description:    QueryGetIndFareBasisMod
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
#include "DBAccess/IndustryFareBasisMod.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetIndFareBasisMod : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetIndFareBasisMod(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetIndFareBasisMod(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetIndFareBasisMod() {};

  virtual const char* getQueryName() const override;

  void findIndustryFareBasisMod(std::vector<tse::IndustryFareBasisMod*>& indFares,
                                const CarrierCode& carrier,
                                const Indicator& userApplType,
                                const UserApplCode& userAppl);

  static void initialize();

  const QueryGetIndFareBasisMod& operator=(const QueryGetIndFareBasisMod& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetIndFareBasisMod& operator=(const std::string& Another)
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
}; // class QueryGetIndFareBasisMod

class QueryGetIndFareBasisModHistorical : public tse::QueryGetIndFareBasisMod
{
public:
  QueryGetIndFareBasisModHistorical(DBAdapter* dbAdapt)
    : QueryGetIndFareBasisMod(dbAdapt, _baseSQL) {};
  virtual ~QueryGetIndFareBasisModHistorical() {};

  virtual const char* getQueryName() const override;

  void findIndustryFareBasisMod(std::vector<tse::IndustryFareBasisMod*>& indFares,
                                const CarrierCode& carrier,
                                const Indicator& userApplType,
                                const UserApplCode& userAppl);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetIndFareBasisModHistorical

class QueryGetAllIndFareBasisMod : public QueryGetIndFareBasisMod
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllIndFareBasisMod(DBAdapter* dbAdapt) : QueryGetIndFareBasisMod(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllIndFareBasisMod() {};
  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::IndustryFareBasisMod*>& indFares)
  {
    findAllIndustryFareBasisMod(indFares);
  }

  void findAllIndustryFareBasisMod(std::vector<tse::IndustryFareBasisMod*>& indFares);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllIndFareBasisMod

class QueryGetAllIndFareBasisModHistorical : public QueryGetIndFareBasisMod
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllIndFareBasisModHistorical(DBAdapter* dbAdapt)
    : QueryGetIndFareBasisMod(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllIndFareBasisModHistorical() {};
  virtual const char* getQueryName() const override;

  void findAllIndustryFareBasisMod(std::vector<tse::IndustryFareBasisMod*>& indFares);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllIndFareBasisModHistorical
} // namespace tse

