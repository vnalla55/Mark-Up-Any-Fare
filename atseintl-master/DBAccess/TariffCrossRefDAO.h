//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/HistoricalDataAccessObject.h"

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class TariffCrossRefInfo;
class TariffCrossRefInfoContainer;
class DeleteList;

typedef HashKey<VendorCode, CarrierCode, RecordScope> TariffCrossRefKey;

class TariffCrossRefDAO : public DataAccessObject<TariffCrossRefKey, TariffCrossRefInfoContainer>
{
public:
  static TariffCrossRefDAO& instance();
  const std::vector<TariffCrossRefInfo*>& get(DeleteList& del,
                                              const VendorCode& vendor,
                                              const CarrierCode& carrier,
                                              const RecordScope& crossRefType,
                                              const DateTime& ticketDate);
  const std::vector<TariffCrossRefInfo*>& getByFareTariff(DeleteList& del,
                                                          const VendorCode& vendor,
                                                          const CarrierCode& carrier,
                                                          const RecordScope& crossRefType,
                                                          const TariffNumber& fareTariff,
                                                          const DateTime& date,
                                                          const DateTime& ticketDate);
  const std::vector<TariffCrossRefInfo*>& getByRuleTariff(DeleteList& del,
                                                          const VendorCode& vendor,
                                                          const CarrierCode& carrier,
                                                          const RecordScope& crossRefType,
                                                          const TariffNumber& ruleTariff,
                                                          const DateTime& date,
                                                          const DateTime& ticketDate);
  const std::vector<TariffCrossRefInfo*>& getByGenRuleTariff(DeleteList& del,
                                                             const VendorCode& vendor,
                                                             const CarrierCode& carrier,
                                                             const RecordScope& crossRefType,
                                                             const TariffNumber& ruleTariff,
                                                             const DateTime& date,
                                                             const DateTime& ticketDate);
  const std::vector<TariffCrossRefInfo*>& getByAddonTariff(DeleteList& del,
                                                           const VendorCode& vendor,
                                                           const CarrierCode& carrier,
                                                           const RecordScope& crossRefType,
                                                           const TariffNumber& addonTariff,
                                                           const DateTime& date,
                                                           const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, TariffCrossRefKey& key) const override
  {
    return key.initialized = objectKey.getValue("VENDOR", key._a) &&
                             objectKey.getValue("CARRIER", key._b) &&
                             objectKey.getValue("TARIFFCROSSREFTYPE", key._c);
  }

  TariffCrossRefKey createKey(TariffCrossRefInfoContainer* info);

  void translateKey(const TariffCrossRefKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("CARRIER", key._b);
    objectKey.setValue("TARIFFCROSSREFTYPE", key._c);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<TariffCrossRefInfoContainer, TariffCrossRefDAO>(flatKey, objectKey)
        .success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<TariffCrossRefDAO>;
  static DAOHelper<TariffCrossRefDAO> _helper;
  TariffCrossRefDAO(int cacheSize = 0, const std::string& cacheType = "")
#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
    : DataAccessObject<TariffCrossRefKey, TariffCrossRefInfoContainer>(cacheSize, cacheType, 3)
#else
    : DataAccessObject<TariffCrossRefKey, TariffCrossRefInfoContainer>(cacheSize, cacheType, 2)
#endif
  {
  }
  void load() override;
  TariffCrossRefInfoContainer* create(TariffCrossRefKey key) override;
  void destroy(TariffCrossRefKey key, TariffCrossRefInfoContainer* t) override;

private:
  struct isEffective;
  struct matchFareTariff;
  struct matchRuleTariff;
  struct matchGenRuleTariff;
  struct matchAddonTariff;
  static TariffCrossRefDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: TariffCrossRefHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, CarrierCode, RecordScope, DateTime, DateTime>
TariffCrossRefHistoricalKey;

class TariffCrossRefHistoricalDAO
    : public HistoricalDataAccessObject<TariffCrossRefHistoricalKey, TariffCrossRefInfoContainer>
{
public:
  static TariffCrossRefHistoricalDAO& instance();
  const std::vector<TariffCrossRefInfo*>& get(DeleteList& del,
                                              const VendorCode& vendor,
                                              const CarrierCode& carrier,
                                              const RecordScope& crossRefType,
                                              const DateTime& ticketDate);
  const std::vector<TariffCrossRefInfo*>& getByFareTariff(DeleteList& del,
                                                          const VendorCode& vendor,
                                                          const CarrierCode& carrier,
                                                          const RecordScope& crossRefType,
                                                          const TariffNumber& fareTariff,
                                                          const DateTime& date,
                                                          const DateTime& ticketDate);
  const std::vector<TariffCrossRefInfo*>& getByRuleTariff(DeleteList& del,
                                                          const VendorCode& vendor,
                                                          const CarrierCode& carrier,
                                                          const RecordScope& crossRefType,
                                                          const TariffNumber& ruleTariff,
                                                          const DateTime& date,
                                                          const DateTime& ticketDate);
  const std::vector<TariffCrossRefInfo*>& getByGenRuleTariff(DeleteList& del,
                                                             const VendorCode& vendor,
                                                             const CarrierCode& carrier,
                                                             const RecordScope& crossRefType,
                                                             const TariffNumber& ruleTariff,
                                                             const DateTime& date,
                                                             const DateTime& ticketDate);
  const std::vector<TariffCrossRefInfo*>& getByAddonTariff(DeleteList& del,
                                                           const VendorCode& vendor,
                                                           const CarrierCode& carrier,
                                                           const RecordScope& crossRefType,
                                                           const TariffNumber& addonTariff,
                                                           const DateTime& date,
                                                           const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, TariffCrossRefHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b) &&
               objectKey.getValue("TARIFFCROSSREFTYPE", key._c) &&
               objectKey.getValue("STARTDATE", key._d) && objectKey.getValue("ENDDATE", key._e);
  }

  bool translateKey(const ObjectKey& objectKey,
                    TariffCrossRefHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
    return key.initialized = objectKey.getValue("VENDOR", key._a) &&
                             objectKey.getValue("CARRIER", key._b) &&
                             objectKey.getValue("TARIFFCROSSREFTYPE", key._c);
  }

  void setKeyDateRange(TariffCrossRefHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<TariffCrossRefHistoricalDAO>;
  static DAOHelper<TariffCrossRefHistoricalDAO> _helper;
  TariffCrossRefHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<TariffCrossRefHistoricalKey, TariffCrossRefInfoContainer>(
          cacheSize, cacheType, 2)
  {
  }
  TariffCrossRefInfoContainer* create(TariffCrossRefHistoricalKey key) override;
  void destroy(TariffCrossRefHistoricalKey key, TariffCrossRefInfoContainer* t) override;

private:
  struct isEffective;
  struct matchFareTariff;
  struct matchRuleTariff;
  struct matchGenRuleTariff;
  struct matchAddonTariff;
  static TariffCrossRefHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
