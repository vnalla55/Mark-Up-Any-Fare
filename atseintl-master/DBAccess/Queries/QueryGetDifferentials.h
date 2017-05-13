//----------------------------------------------------------------------------
//          File:           QueryGetDifferentials.h
//          Description:    QueryGetDifferentials
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
#include "DBAccess/Differentials.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{

class QueryGetDifferentials : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetDifferentials(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetDifferentials(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetDifferentials() {};

  virtual const char* getQueryName() const override;

  void findDifferentials(std::vector<tse::Differentials*>& diffs, const CarrierCode& cxr);

  static void initialize();

  const QueryGetDifferentials& operator=(const QueryGetDifferentials& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetDifferentials& operator=(const std::string& Another)
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
}; // class QueryGetDifferentials

// GetHistorical
class QueryGetDifferentialsHistorical : public QueryGetDifferentials
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetDifferentialsHistorical(DBAdapter* dbAdapt) : QueryGetDifferentials(dbAdapt, _baseSQL) {};
  virtual const char* getQueryName() const override;
  void findDifferentials(std::vector<tse::Differentials*>& diffs, const CarrierCode& cxr);
  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetDifferentialsHistorical

// GetAll
class QueryGetAllDifferentials : public QueryGetDifferentials
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllDifferentials(DBAdapter* dbAdapt) : QueryGetDifferentials(dbAdapt, _baseSQL) {};
  virtual const char* getQueryName() const override;
  void execute(std::vector<tse::Differentials*>& diffs) { findAllDifferentials(diffs); }

  void findAllDifferentials(std::vector<tse::Differentials*>& diffs);
  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllDifferentials

// GetAllHistorical
class QueryGetAllDifferentialsHistorical : public QueryGetDifferentials
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllDifferentialsHistorical(DBAdapter* dbAdapt)
    : QueryGetDifferentials(dbAdapt, _baseSQL) {};
  virtual const char* getQueryName() const override;
  void findAllDifferentials(std::vector<tse::Differentials*>& diffs);
  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllDifferentialsHistorical
} // namespace tse

