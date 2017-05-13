//----------------------------------------------------------------------------
//          File:           QueryGetTariffMileageAddon.h
//          Description:    QueryGetTariffMileageAddon
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
#include "DBAccess/TariffMileageAddon.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetTariffMileageAddon : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTariffMileageAddon(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetTariffMileageAddon(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetTariffMileageAddon() {}

  virtual const char* getQueryName() const override;

  void findTariffMileageAddon(std::vector<tse::TariffMileageAddon*>& lstTMA,
                              const CarrierCode& carrier,
                              const LocCode& unpublishedAddonLoc,
                              const GlobalDirection& globalDir);

  static void initialize();

  const QueryGetTariffMileageAddon& operator=(const QueryGetTariffMileageAddon& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTariffMileageAddon& operator=(const std::string& Another)
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
}; // class QueryGetTariffMileageAddon

class QueryGetTariffMileageAddonHistorical : public tse::QueryGetTariffMileageAddon
{
public:
  QueryGetTariffMileageAddonHistorical(DBAdapter* dbAdapt)
    : QueryGetTariffMileageAddon(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetTariffMileageAddonHistorical() {}
  virtual const char* getQueryName() const override;

  void findTariffMileageAddon(std::vector<tse::TariffMileageAddon*>& lstTMA,
                              const CarrierCode& carrier,
                              const LocCode& unpublishedAddonLoc,
                              const GlobalDirection& globalDir);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetTariffMileageAddonHistorical

class QueryGetAllTariffMileageAddon : public QueryGetTariffMileageAddon
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllTariffMileageAddon(DBAdapter* dbAdapt) : QueryGetTariffMileageAddon(dbAdapt, _baseSQL)
  {
  }

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::TariffMileageAddon*>& lstTMA) { findAllTariffMileageAddon(lstTMA); }

  void findAllTariffMileageAddon(std::vector<tse::TariffMileageAddon*>& lstTMA);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllTariffMileageAddon

class QueryGetAllTariffMileageAddonHistorical : public QueryGetTariffMileageAddon
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllTariffMileageAddonHistorical(DBAdapter* dbAdapt)
    : QueryGetTariffMileageAddon(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetAllTariffMileageAddonHistorical() {}
  virtual const char* getQueryName() const override;

  void findAllTariffMileageAddon(std::vector<tse::TariffMileageAddon*>& lstTMA);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllTariffMileageAddonHistorical
} // namespace tse

