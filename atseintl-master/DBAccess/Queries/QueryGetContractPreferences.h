//----------------------------------------------------------------------------
//          File:           QueryGetContractPreferences.h
//          Description:    QueryGetContractPreferences
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
#include "DBAccess/ContractPreference.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetContractPreferences : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetContractPreferences(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetContractPreferences(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetContractPreferences() {};

  virtual const char* getQueryName() const override;

  void findContractPreferences(std::vector<ContractPreference*>& cpList,
                               const PseudoCityCode& pseudoCity,
                               const CarrierCode& carrier,
                               const RuleNumber& rule);

  static void initialize();

  const QueryGetContractPreferences& operator=(const QueryGetContractPreferences& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetContractPreferences& operator=(const std::string& Another)
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
}; // class QueryGetContractPreferences
class QueryGetContractPreferencesHistorical : public tse::QueryGetContractPreferences
{
public:
  QueryGetContractPreferencesHistorical(DBAdapter* dbAdapt)
    : QueryGetContractPreferences(dbAdapt, _baseSQL) {};
  virtual ~QueryGetContractPreferencesHistorical() {};
  virtual const char* getQueryName() const override;

  void findContractPreferences(std::vector<ContractPreference*>& cpList,
                               const PseudoCityCode& pseudoCity,
                               const CarrierCode& carrier,
                               const RuleNumber& rule);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetContractPreferencesHistorical
class QueryGetAllContractPreferencesHistorical : public tse::QueryGetContractPreferences
{
public:
  QueryGetAllContractPreferencesHistorical(DBAdapter* dbAdapt)
    : QueryGetContractPreferences(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllContractPreferencesHistorical() {};
  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::ContractPreference*>& lstCP) { findAllContractPreferences(lstCP); }

  void findAllContractPreferences(std::vector<tse::ContractPreference*>& lstCP);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllContractPreferencesHistorical
} // namespace tse

