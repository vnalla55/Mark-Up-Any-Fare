//----------------------------------------------------------------------------
//          File:           QueryGetFareCalcConfigs.h
//          Description:    QueryGetFareCalcConfigs
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
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetFareCalcConfigs : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareCalcConfigs(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetFareCalcConfigs(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetFareCalcConfigs() {};

  virtual const char* getQueryName() const override;

  void findFareCalcConfigs(std::vector<tse::FareCalcConfig*>& fcCfgs,
                           const Indicator userApplType,
                           const UserApplCode& userAppl,
                           const PseudoCityCode& pseudoCity);

  static void initialize();

  const QueryGetFareCalcConfigs& operator=(const QueryGetFareCalcConfigs& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetFareCalcConfigs& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  };
  static int stringToInteger(const char* stringVal, int lineNumber);

protected:
private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetFareCalcConfigs

class QueryGetAllFareCalcConfigs : public QueryGetFareCalcConfigs
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllFareCalcConfigs(DBAdapter* dbAdapt) : QueryGetFareCalcConfigs(dbAdapt, _baseSQL) {};

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::FareCalcConfig*>& fcCfgs) { findAllFareCalcConfigs(fcCfgs); }

  void findAllFareCalcConfigs(std::vector<tse::FareCalcConfig*>& fcCfgs);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllFareCalcConfigs
} // namespace tse

