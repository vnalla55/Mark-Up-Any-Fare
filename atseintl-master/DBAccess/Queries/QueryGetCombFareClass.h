//----------------------------------------------------------------------------
//              File:           QueryGetCombFareClass.h
//              Description:    QueryGetCombFareClass
//              Created:        3/2/2006
//     Authors:         Mike Lillis
//
//              Updates:
//
//         � 2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//         and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//         or transfer of this software/documentation, in any medium, or incorporation of this
//         software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "DBAccess/AddonCombFareClassInfo.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{

class QueryGetCombFareClass : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCombFareClass(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetCombFareClass(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetCombFareClass() {};
  virtual const char* getQueryName() const override;

  void findAddonCombFareClassInfo(AddonFareClassCombMultiMap& addonCombs,
                                  const VendorCode& vendor,
                                  const TariffNumber& fareTariff,
                                  const CarrierCode& carrier);

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION

  static bool complementGeoAppl(const tse::AddonCombFareClassInfo* newRec,
                                AddonFareClassCombMultiMap::mapped_type& prevStored);

#endif

  static void initialize();

  const QueryGetCombFareClass& operator=(const QueryGetCombFareClass& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetCombFareClass& operator=(const std::string& Another)
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
}; // class QueryGetCombFareClass

class QueryGetCombFareClassHistorical : public QueryGetCombFareClass
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCombFareClassHistorical(DBAdapter* dbAdapt) : QueryGetCombFareClass(dbAdapt, _baseSQL) {};
  virtual ~QueryGetCombFareClassHistorical() {};
  virtual const char* getQueryName() const override;

  void findAddonCombFareClassInfo(std::vector<AddonCombFareClassInfo*>& addonCombs,
                                  const VendorCode& vendor,
                                  const TariffNumber& fareTariff,
                                  const CarrierCode& carrier,
                                  const DateTime& startDate,
                                  const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetCombFareClassHistorical

class QueryGetAllCombFareClass : public QueryGetCombFareClass
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllCombFareClass(DBAdapter* dbAdapt) : QueryGetCombFareClass(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllCombFareClass() {};
  virtual const char* getQueryName() const override;

  void execute(AddonFareClassVCCombMap& addonCombs) { findAllAddonCombFareClassInfo(addonCombs); }

  void findAllAddonCombFareClassInfo(AddonFareClassVCCombMap& addonCombs);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
} // namespace tse
