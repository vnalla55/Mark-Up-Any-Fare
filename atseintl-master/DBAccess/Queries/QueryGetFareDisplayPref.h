//----------------------------------------------------------------------------
//          File:           QueryGetFareDisplayPref.h
//          Description:    QueryGetFareDisplayPref
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
#include "DBAccess/FareDisplayPref.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetFareDisplayPref : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareDisplayPref(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetFareDisplayPref(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetFareDisplayPref() {};

  virtual const char* getQueryName() const override;

  void findFareDisplayPref(std::vector<tse::FareDisplayPref*>& fareDisplayPrefs,
                           const Indicator& userApplType,
                           const UserApplCode& userAppl,
                           const Indicator& pseudoCityType,
                           const PseudoCityCode& pseudoCity,
                           const TJRGroup& tjrGroup);

  static void initialize();

  const QueryGetFareDisplayPref& operator=(const QueryGetFareDisplayPref& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetFareDisplayPref& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  };

protected:
private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetFareDisplayPref

class QueryGetAllFareDisplayPref : public QueryGetFareDisplayPref
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllFareDisplayPref(DBAdapter* dbAdapt) : QueryGetFareDisplayPref(dbAdapt, _baseSQL) {};

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::FareDisplayPref*>& fareDisplayPrefs)
  {
    findAllFareDisplayPref(fareDisplayPrefs);
  }

  void findAllFareDisplayPref(std::vector<tse::FareDisplayPref*>& fareDisplayPrefs);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllFareDisplayPref
} // namespace tse

