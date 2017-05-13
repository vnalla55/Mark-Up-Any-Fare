//----------------------------------------------------------------------------
//          File:           QueryGetAddonZones.h
//          Description:    QueryGetAddonZones
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
#include "DBAccess/AddonZoneInfo.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{

class QueryGetAddonZones : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAddonZones(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetAddonZones(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetAddonZones() {};

  virtual const char* getQueryName() const override;

  void findAddonZoneInfo(std::vector<tse::AddonZoneInfo*>& addZones,
                         const VendorCode& vendor,
                         const CarrierCode& carrier,
                         const LocCode& market);

  static void initialize();

  const QueryGetAddonZones& operator=(const QueryGetAddonZones& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetAddonZones& operator=(const std::string& Another)
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
}; // class QueryGetAddonZones

class QueryGetAddonZoneInfo : public QueryGetAddonZones
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAddonZoneInfo(DBAdapter* dbAdapt) : QueryGetAddonZones(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAddonZoneInfo() {};
  virtual const char* getQueryName() const override;

  void findAddonZoneInfo(std::vector<tse::AddonZoneInfo*>& addZones,
                         const VendorCode& vendor,
                         const CarrierCode& carrier,
                         const TariffNumber& fareTariff,
                         const AddonZone& zone);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAddonZoneInfo

class QueryGetAllAddonZones : public QueryGetAddonZones
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllAddonZones(DBAdapter* dbAdapt) : QueryGetAddonZones(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllAddonZones() {};
  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::AddonZoneInfo*>& addZones) { findAllAddonZoneInfo(addZones); }

  void findAllAddonZoneInfo(std::vector<tse::AddonZoneInfo*>& addZones);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllAddonZones

class QueryGetAddonZonesHistorical : public tse::QueryGetAddonZones
{
public:
  QueryGetAddonZonesHistorical(DBAdapter* dbAdapt) : QueryGetAddonZones(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAddonZonesHistorical() {};
  virtual const char* getQueryName() const override;

  void findAddonZoneInfo(std::vector<tse::AddonZoneInfo*>& addZones,
                         const VendorCode& vendor,
                         const CarrierCode& carrier,
                         const LocCode& market,
                         const DateTime& startDate,
                         const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;

  void setAdapterAndBaseSQL(DBAdapter* dbAdapt);
}; // class QueryGetAddonZonesHistorical

class QueryGetAddonZoneInfoHistorical : public tse::QueryGetAddonZones
{
public:
  QueryGetAddonZoneInfoHistorical(DBAdapter* dbAdapt) : QueryGetAddonZones(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAddonZoneInfoHistorical() {};
  virtual const char* getQueryName() const override;

  void findAddonZoneInfo(std::vector<tse::AddonZoneInfo*>& addZones,
                         const VendorCode& vendor,
                         const CarrierCode& carrier,
                         const TariffNumber& fareTariff,
                         const AddonZone& zone,
                         const DateTime& startDate,
                         const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;

  void setAdapterAndBaseSQL(DBAdapter* dbAdapt);
}; // class QueryGetAddonZoneInfoHistorical

} // namespace tse

