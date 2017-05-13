//----------------------------------------------------------------------------
//          File:           QueryGetPfcEquipTypeExempt.h
//          Description:    QueryGetPfcEquipTypeExempt
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
#include "DBAccess/PfcEquipTypeExempt.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetPfcEquipTypeExempt : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetPfcEquipTypeExempt(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetPfcEquipTypeExempt(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetPfcEquipTypeExempt() {}

  virtual const char* getQueryName() const override;

  void findPfcEquipTypeExempt(std::vector<tse::PfcEquipTypeExempt*>& lstETE,
                              const EquipmentType& equip,
                              const StateCode& state);

  static void initialize();
  const QueryGetPfcEquipTypeExempt& operator=(const QueryGetPfcEquipTypeExempt& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetPfcEquipTypeExempt& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  }

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetPfcEquipTypeExempt

class QueryGetAllPfcEquipTypeExempt : public QueryGetPfcEquipTypeExempt
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllPfcEquipTypeExempt(DBAdapter* dbAdapt) : QueryGetPfcEquipTypeExempt(dbAdapt, _baseSQL)
  {
  }

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::PfcEquipTypeExempt*>& lstETE) { findAllPfcEquipTypeExempt(lstETE); }

  void findAllPfcEquipTypeExempt(std::vector<tse::PfcEquipTypeExempt*>& lstETE);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllPfcEquipTypeExempt

class QueryGetPfcEquipTypeExemptHistorical : public QueryGetPfcEquipTypeExempt
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetPfcEquipTypeExemptHistorical(DBAdapter* dbAdapt)
    : QueryGetPfcEquipTypeExempt(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetPfcEquipTypeExemptHistorical() {}

  virtual const char* getQueryName() const override;

  void findPfcEquipTypeExempt(std::vector<tse::PfcEquipTypeExempt*>& lstETE,
                              const EquipmentType& equip,
                              const StateCode& state,
                              const DateTime& startDate,
                              const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetPfcEquipTypeExemptHistorical

} // namespace tse

