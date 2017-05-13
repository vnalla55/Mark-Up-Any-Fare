//----------------------------------------------------------------------------
//  File:           QueryGetCombFareClass.cpp
//  Description:    QueryGetCombFareClass
//  Created:        5/24/2006
//  Authors:         Mike Lillis
//
//  Updates:
//
//  2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//  and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//  or transfer of this software/documentation, in any medium, or incorporation of this
//  software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetCombFareClass.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCombFareClassSQLStatement.h"

#include <boost/move/move.hpp>

namespace tse
{
log4cxx::LoggerPtr
QueryGetCombFareClass::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetCombFareClass"));
std::string QueryGetCombFareClass::_baseSQL;
bool QueryGetCombFareClass::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCombFareClass> g_GetCombFareClass;

const char*
QueryGetCombFareClass::getQueryName() const
{
  return "GETCOMBFARECLASS";
}

void
QueryGetCombFareClass::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCombFareClassSQLStatement<QueryGetCombFareClass> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCOMBFARECLASS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION

bool
QueryGetCombFareClass::complementGeoAppl(const tse::AddonCombFareClassInfo* newRec,
                                         AddonFareClassCombMultiMap::mapped_type& prevStored)
{
  for (auto stored: prevStored)
  {
    // There are two distinct possible non-blank GeoAppl values: 'N' or 'U'.
    // In case two different values are read from DB for two otherwise identical records,
    // corresponding AddonCombFareClassInfo objects are collapsed into one with blank geoAppl
    // matching both cases.
    // In case any AddonCombFareClass record has blank GeoAppl already read from DB,
    // the result is also single AddonCombFareClassInfo object with blank geoAppl.
    if (stored->addonFareClass() == newRec->addonFareClass() &&
        stored->geoAppl() != newRec->geoAppl())
    {
      stored->geoAppl() = ' ';
      return true;
    }
  }
  return false;
}

void
QueryGetCombFareClass::findAddonCombFareClassInfo(AddonFareClassCombMultiMap& addonCombs,
                                                  const VendorCode& vendor,
                                                  const TariffNumber& fareTariff,
                                                  const CarrierCode& carrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, vendor);
  substParm(2, fareTariff);
  substParm(3, carrier);
  substCurrentDate(false/*use whole timestamp, not only date part*/);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());

  res.executeQuery(this);
  tse::AddonCombFareClassInfo* addon = nullptr;
  while ((row = res.nextRow()))
  {
    addon = QueryGetCombFareClassSQLStatement<
      QueryGetCombFareClass>::mapRowToAddonCombFareClassInfo(row).second;
    if (LIKELY(addon))
    {
      AddonCombFareClassSpecifiedKey key(addon->fareClass(), addon->owrt());
      AddonFareClassCombMultiMap::mapped_type& pr(addonCombs[key]);
      if (!complementGeoAppl(addon, pr))
        pr.push_back(addon);
      else
      {
        // New AddonCombFareClassInfo object was merged with existing one -
        // identical except for geoAppl.
        delete addon;
      }
    }
  }
  LOG4CXX_INFO(_logger,
               "GETCOMBFARECLASS('" << vendor << "','" << fareTariff << "'," << carrier
                                    << "'): " << res.numRows() << " rows Time = " << stopTimer()
                                    << " (" << stopCPU() << ") mSecs");
  res.freeResult();
}

void
QueryGetCombFareClassHistorical::findAddonCombFareClassInfo(
    std::vector<AddonCombFareClassInfo*>& addonCombs,
    const VendorCode& vendor,
    const TariffNumber& fareTariff,
    const CarrierCode& carrier,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, vendor);
  substParm(2, fareTariff);
  substParm(3, carrier);
  substParm(4, startDate);
  substParm(5, endDate);
  substParm(6, endDate);
  substParm(7, vendor);
  substParm(8, fareTariff);
  substParm(9, carrier);
  substParm(10, startDate);
  substParm(11, endDate);
  substParm(12, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    tse::AddonCombFareClassInfo* addon(QueryGetCombFareClassHistoricalSQLStatement<
        QueryGetCombFareClassHistorical>::mapRowToAddonCombFareClassInfo(row).second);
    if (addon)
    {
      addonCombs.push_back(addon);
    }
  }
  LOG4CXX_INFO(_logger,
               "GETCOMBFARECLASSHISTORICAL('"
                   << vendor << "','" << fareTariff << "'," << carrier << "'): " << res.numRows()
                   << " rows Time = " << stopTimer() << " (" << stopCPU() << ") mSecs");
  res.freeResult();
}

#else

void
QueryGetCombFareClass::findAddonCombFareClassInfo(AddonFareClassCombMultiMap& addonCombs,
                                                  const VendorCode& vendor,
                                                  const TariffNumber& fareTariff,
                                                  const CarrierCode& carrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, vendor);
  substParm(2, fareTariff);
  substParm(3, carrier);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());

  res.executeQuery(this);
  tse::AddonCombFareClassInfo* addon = nullptr;
  while ((row = res.nextRow()))
  {
    addon =
        QueryGetCombFareClassSQLStatement<QueryGetCombFareClass>::mapRowToAddonCombFareClassInfo(
            row);
    if (LIKELY(addon))
    {
      AddonCombFareClassInfoKey key(
          addon->addonFareClass(), addon->geoAppl(), addon->owrt(), addon->fareClass());
      AddonFareClassCombMultiMap::value_type pr(key, addon);
      addonCombs.insert(boost::move(pr));
    }
  }
  LOG4CXX_INFO(_logger,
               "GETCOMBFARECLASS('" << vendor << "','" << fareTariff << "'," << carrier
                                    << "'): " << res.numRows() << " rows Time = " << stopTimer()
                                    << " (" << stopCPU() << ") mSecs");
  res.freeResult();
}

void
QueryGetCombFareClassHistorical::findAddonCombFareClassInfo(
    std::vector<AddonCombFareClassInfo*>& addonCombs,
    const VendorCode& vendor,
    const TariffNumber& fareTariff,
    const CarrierCode& carrier,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, vendor);
  substParm(2, fareTariff);
  substParm(3, carrier);
  substParm(4, startDate);
  substParm(5, endDate);
  substParm(6, endDate);
  substParm(7, vendor);
  substParm(8, fareTariff);
  substParm(9, carrier);
  substParm(10, startDate);
  substParm(11, endDate);
  substParm(12, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    tse::AddonCombFareClassInfo* addon(QueryGetCombFareClassHistoricalSQLStatement<
        QueryGetCombFareClassHistorical>::mapRowToAddonCombFareClassInfo(row));
    if (addon)
    {
      addonCombs.push_back(addon);
    }
  }
  LOG4CXX_INFO(_logger,
               "GETCOMBFARECLASSHISTORICAL('"
                   << vendor << "','" << fareTariff << "'," << carrier << "'): " << res.numRows()
                   << " rows Time = " << stopTimer() << " (" << stopCPU() << ") mSecs");
  res.freeResult();
}

#endif

///////////////////////////////////////////////////////////
//  QueryGetCombFareClassHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetCombFareClassHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetCombFareClassHistorical"));
std::string QueryGetCombFareClassHistorical::_baseSQL;
bool QueryGetCombFareClassHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCombFareClassHistorical> g_GetCombFareClassHistorical;

const char*
QueryGetCombFareClassHistorical::getQueryName() const
{
  return "GETCOMBFARECLASSHISTORICAL";
}

void
QueryGetCombFareClassHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCombFareClassHistoricalSQLStatement<QueryGetCombFareClassHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCOMBFARECLASSHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

///////////////////////////////////////////////////////////
//  QueryGetAllCombFareClass
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllCombFareClass::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllCombFareClass"));
std::string QueryGetAllCombFareClass::_baseSQL;
bool QueryGetAllCombFareClass::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllCombFareClass> g_GetAllCombFareClass;

const char*
QueryGetAllCombFareClass::getQueryName() const
{
  return "GETALLCOMBFARECLASS";
}
void
QueryGetAllCombFareClass::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllCombFareClassSQLStatement<QueryGetAllCombFareClass> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLCOMBFARECLASS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION

void
QueryGetAllCombFareClass::findAllAddonCombFareClassInfo(AddonFareClassVCCombMap& addonCombs)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate(false/*use whole timestamp, not only date part*/);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  AddonCombFareClassInfoKey1 prevKey;
  AddonFareClassCombMultiMap* addonMultiMap = nullptr;

  while ((row = res.nextRow()))
  {
    const std::pair<AddonCombFareClassInfoKey1,
                    AddonCombFareClassInfo*>& addon = QueryGetAllCombFareClassSQLStatement<
                      QueryGetAllCombFareClass>::mapRowToAddonCombFareClassInfo(row);

    if (LIKELY(addon.second))
    {
      // IF this is a new Vendor/Tariff/Carrier
      if (!(addon.first == prevKey))
      {
        // Create a new multimap for this Vendor/Tariff/Carrier and set it as the active one
        addonMultiMap = new AddonFareClassCombMultiMap;
        addonCombs.insert(AddonFareClassVCCombMap::value_type(addon.first, addonMultiMap));
        prevKey = addon.first;
      }
      // Insert the new object into the multimap
      AddonCombFareClassSpecifiedKey key(addon.second->fareClass(), addon.second->owrt());
      AddonFareClassCombMultiMap::mapped_type& pr((*addonMultiMap)[key]);
      if (!complementGeoAppl(addon.second, pr))
        pr.push_back(addon.second);
      else
      {
        // New AddonCombFareClassInfo object was merged with existing one -
        // identical except for geoAppl.
        delete addon.second;
      }
    }
  }
  LOG4CXX_INFO(_logger,
               "GETALLCOMBFARECLASS: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                 << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetAllCombFareClass::findAllAddonCombFareClassInfo()

#else

void
QueryGetAllCombFareClass::findAllAddonCombFareClassInfo(AddonFareClassVCCombMap& addonCombs)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  tse::AddonCombFareClassInfo* addon = nullptr;

  VendorCode prevVendor;
  TariffNumber prevTariff(0);
  CarrierCode prevCarrier;
  AddonFareClassCombMultiMap* addonMultiMap = nullptr;

  while ((row = res.nextRow()))
  {
    addon = QueryGetAllCombFareClassSQLStatement<
        QueryGetAllCombFareClass>::mapRowToAddonCombFareClassInfo(row);

    if (LIKELY(addon))
    {
      // IF this is a new Vendor/Tariff/Carrier
      if ((addon->vendor() != prevVendor) || (addon->fareTariff() != prevTariff) ||
          (addon->carrier() != prevCarrier))
      {
        // Create a new multimap for this Vendor/Tariff/Carrier and set it as the active one
        addonMultiMap = new AddonFareClassCombMultiMap;

        AddonCombFareClassInfoKey1 vendorCarrierKey(
            addon->vendor(), addon->fareTariff(), addon->carrier());

        addonCombs.insert(AddonFareClassVCCombMap::value_type(vendorCarrierKey, addonMultiMap));

        prevVendor = addon->vendor();
        prevTariff = addon->fareTariff();
        prevCarrier = addon->carrier();
      }
      // Insert the new object into the multimap
      AddonCombFareClassInfoKey key(
          addon->addonFareClass(), addon->geoAppl(), addon->owrt(), addon->fareClass());
      AddonFareClassCombMultiMap::value_type pr(key, addon);
      addonMultiMap->insert(boost::move(pr));
    }
  }
  LOG4CXX_INFO(_logger,
               "GETALLCOMBFARECLASS: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                 << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetAllCombFareClass::findAllAddonCombFareClassInfo()

#endif

}
