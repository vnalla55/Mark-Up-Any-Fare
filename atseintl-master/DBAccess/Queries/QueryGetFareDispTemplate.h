//----------------------------------------------------------------------------
//          File:           QueryGetFareDispTemplate.h
//          Description:    QueryGetFareDispTemplate
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
#include "DBAccess/FareDispTemplate.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetFareDispTemplate : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareDispTemplate(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetFareDispTemplate(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetFareDispTemplate() {};

  virtual const char* getQueryName() const override;

  void findFareDispTemplate(std::vector<tse::FareDispTemplate*>& infos,
                            const int& templateID,
                            const Indicator& templateType);

  static void initialize();

  const QueryGetFareDispTemplate& operator=(const QueryGetFareDispTemplate& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetFareDispTemplate& operator=(const std::string& Another)
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
}; // class QueryGetFareDispTemplate

class QueryGetAllFareDispTemplate : public QueryGetFareDispTemplate
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllFareDispTemplate(DBAdapter* dbAdapt) : QueryGetFareDispTemplate(dbAdapt, _baseSQL) {};

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::FareDispTemplate*>& infos) { findAllFareDispTemplate(infos); }

  void findAllFareDispTemplate(std::vector<tse::FareDispTemplate*>& infos);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllFareDispTemplate
} // namespace tse

