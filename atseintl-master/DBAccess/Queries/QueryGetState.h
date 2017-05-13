//----------------------------------------------------------------------------
//          File:           QueryGetState.h
//          Description:    QueryGetState
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
#include "DBAccess/SQLQuery.h"
#include "DBAccess/State.h"

namespace tse
{

class QueryGetState : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetState(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetState(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetState() {};
  virtual const char* getQueryName() const override;

  void
  findState(std::vector<tse::State*>& states, const NationCode& nation, const StateCode& state);

  static void initialize();

  const QueryGetState& operator=(const QueryGetState& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetState& operator=(const std::string& Another)
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
}; // class QueryGetState

class QueryGetStateHistorical : public QueryGetState
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetStateHistorical(DBAdapter* dbAdapt) : QueryGetState(dbAdapt, _baseSQL) {}
  virtual ~QueryGetStateHistorical() {};
  virtual const char* getQueryName() const override;

  void
  findState(std::vector<tse::State*>& states, const NationCode& nation, const StateCode& state);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetStateHistorical

class QueryGetStates : public QueryGetState
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetStates(DBAdapter* dbAdapt) : QueryGetState(dbAdapt, _baseSQL) {};
  virtual ~QueryGetStates() {};
  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::State*>& states) { findAllStates(states); }

  void findAllStates(std::vector<tse::State*>& states);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetStates

class QueryGetStatesHistorical : public QueryGetState
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetStatesHistorical(DBAdapter* dbAdapt) : QueryGetState(dbAdapt, _baseSQL) {}
  virtual ~QueryGetStatesHistorical() {};
  virtual const char* getQueryName() const override;

  void findAllStates(std::vector<tse::State*>& states);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetStatesHistorical
} // namespace tse
