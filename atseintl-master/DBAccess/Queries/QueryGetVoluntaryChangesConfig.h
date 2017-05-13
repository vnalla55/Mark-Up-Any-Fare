//----------------------------------------------------------------------------
//          File:           QueryGetVoluntaryChangesConfig.h
//          Description:    QueryGetVoluntaryChangesConfig
//          Created:        10/27/2008
// Authors:         Dean Van Decker
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
#include "DBAccess/VoluntaryChangesConfig.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetVoluntaryChangesConfig : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetVoluntaryChangesConfig(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetVoluntaryChangesConfig(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetVoluntaryChangesConfig() {};

  virtual const char* getQueryName() const override;

  void findVoluntaryChangesConfig(std::vector<tse::VoluntaryChangesConfig*>& voluntaryChangesConfig,
                                  const CarrierCode& carrier);

  static void initialize();

  const QueryGetVoluntaryChangesConfig& operator=(const QueryGetVoluntaryChangesConfig& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetVoluntaryChangesConfig& operator=(const std::string& Another)
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
}; // class QueryGetVoluntaryChangesConfig

class QueryGetAllVoluntaryChangesConfig : public QueryGetVoluntaryChangesConfig
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllVoluntaryChangesConfig(DBAdapter* dbAdapt)
    : QueryGetVoluntaryChangesConfig(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllVoluntaryChangesConfig() {};
  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::VoluntaryChangesConfig*>& voluntaryChangesConfig)
  {
    findAllVoluntaryChangesConfigs(voluntaryChangesConfig);
  }

  void
  findAllVoluntaryChangesConfigs(std::vector<tse::VoluntaryChangesConfig*>& voluntaryChangesConfig);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllVoluntaryChangesConfig

} // namespace tse

