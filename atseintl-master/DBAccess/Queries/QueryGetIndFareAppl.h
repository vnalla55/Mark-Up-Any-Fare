//----------------------------------------------------------------------------
//          File:           QueryGetIndFareAppl.h
//          Description:    QueryGetIndFareAppl
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
#include "DBAccess/IndustryFareAppl.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{

class QueryGetIndFareAppl : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetIndFareAppl(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetIndFareAppl(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetIndFareAppl() {};
  virtual const char* getQueryName() const override;

  void findIndustryFareAppl(std::vector<tse::IndustryFareAppl*>& indFares,
                            const Indicator& selectionType,
                            const CarrierCode& carrier);

  static void initialize();

  const QueryGetIndFareAppl& operator=(const QueryGetIndFareAppl& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetIndFareAppl& operator=(const std::string& Another)
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
}; // class QueryGetIndFareAppl

class QueryGetIndFareApplHistorical : public QueryGetIndFareAppl
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetIndFareApplHistorical(DBAdapter* dbAdapt) : QueryGetIndFareAppl(dbAdapt, _baseSQL) {}
  virtual ~QueryGetIndFareApplHistorical() {}
  virtual const char* getQueryName() const override;

  void findIndustryFareAppl(std::vector<tse::IndustryFareAppl*>& indFares,
                            const Indicator& selectionType,
                            const CarrierCode& carrier,
                            const DateTime& startDate,
                            const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetIndFareApplHistorical

class QueryGetAllIndFareAppl : public QueryGetIndFareAppl
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllIndFareAppl(DBAdapter* dbAdapt) : QueryGetIndFareAppl(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllIndFareAppl() {}
  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::IndustryFareAppl*>& indFares) { findAllIndustryFareAppl(indFares); }

  void findAllIndustryFareAppl(std::vector<tse::IndustryFareAppl*>& indFares);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllIndFareAppl

class QueryGetAllIndFareApplHistorical : public QueryGetIndFareAppl
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllIndFareApplHistorical(DBAdapter* dbAdapt) : QueryGetIndFareAppl(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllIndFareApplHistorical() {}
  virtual const char* getQueryName() const override;

  void findAllIndustryFareAppl(std::vector<tse::IndustryFareAppl*>& indFares);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllIndFareApplHistorical

} // namespace tse

