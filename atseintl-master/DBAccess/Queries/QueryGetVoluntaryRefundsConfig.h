//-----------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly
// prohibited.
//-----------------------------------------------------------------------------

#pragma once

#include "Common/Logger.h"
#include "DBAccess/SQLQuery.h"
#include "DBAccess/VoluntaryRefundsConfig.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{

class QueryGetVoluntaryRefundsConfig : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetVoluntaryRefundsConfig(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}

  QueryGetVoluntaryRefundsConfig(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }

  virtual const char* getQueryName() const override { return "GETVOLUNTARYREFUNDSCONFIG"; }

  void find(std::vector<VoluntaryRefundsConfig*>& lst, const CarrierCode& carrier);

  static void initialize();

  const QueryGetVoluntaryRefundsConfig& operator=(const QueryGetVoluntaryRefundsConfig& Another)
  {
    if (this != &Another)
    {
      SQLQuery::operator=(Another);
    }
    return *this;
  };

  const QueryGetVoluntaryRefundsConfig& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      SQLQuery::operator=(Another);
    }
    return *this;
  };

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

class QueryGetAllVoluntaryRefundsConfig : public QueryGetVoluntaryRefundsConfig
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllVoluntaryRefundsConfig(DBAdapter* dbAdapt)
    : QueryGetVoluntaryRefundsConfig(dbAdapt, _baseSQL)
  {
  }

  virtual const char* getQueryName() const override { return "GETALLVOLUNTARYREFUNDSCONFIG"; }

  void execute(std::vector<VoluntaryRefundsConfig*>& lst) { find(lst); }

  void find(std::vector<VoluntaryRefundsConfig*>& lst);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
}

