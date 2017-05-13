//----------------------------------------------------------------------------
//          File:           QueryGetMileageSubstitution.h
//          Description:    QueryGetMileageSubstitution
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
#include "DBAccess/MileageSubstitution.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetMileageSubstitution : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMileageSubstitution(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetMileageSubstitution(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetMileageSubstitution() {};

  virtual const char* getQueryName() const override;

  void findMileageSubstitution(std::vector<tse::MileageSubstitution*>& lstMS,
                               const LocCode& substitutionLoc);

  static void initialize();

  const QueryGetMileageSubstitution& operator=(const QueryGetMileageSubstitution& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetMileageSubstitution& operator=(const std::string& Another)
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
}; // class QueryGetMileageSubstitution

class QueryGetMileageSubstitutionHistorical : public tse::QueryGetMileageSubstitution
{
public:
  QueryGetMileageSubstitutionHistorical(DBAdapter* dbAdapt)
    : QueryGetMileageSubstitution(dbAdapt, _baseSQL) {};
  virtual ~QueryGetMileageSubstitutionHistorical() {};
  virtual const char* getQueryName() const override;

  void findMileageSubstitution(std::vector<tse::MileageSubstitution*>& lstMS,
                               const LocCode& substitutionLoc);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetMileageSubstitutionHistorical

class QueryGetAllMileageSubstitution : public QueryGetMileageSubstitution
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllMileageSubstitution(DBAdapter* dbAdapt)
    : QueryGetMileageSubstitution(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllMileageSubstitution() {};
  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::MileageSubstitution*>& lstMS) { findAllMileageSubstitution(lstMS); }

  void findAllMileageSubstitution(std::vector<tse::MileageSubstitution*>& lstMS);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllMileageSubstitution

class QueryGetAllMileageSubstitutionHistorical : public QueryGetMileageSubstitution
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllMileageSubstitutionHistorical(DBAdapter* dbAdapt)
    : QueryGetMileageSubstitution(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllMileageSubstitutionHistorical() {};
  virtual const char* getQueryName() const override;

  void findAllMileageSubstitution(std::vector<tse::MileageSubstitution*>& lstMS);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllMileageSubstitutionHistorical
} // namespace tse

