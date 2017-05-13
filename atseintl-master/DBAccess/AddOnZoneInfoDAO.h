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
class AddonZoneInfo;
class DeleteList;

typedef HashKey<VendorCode, CarrierCode, TariffNumber, AddonZone> AddOnZoneInfoKey;

class AddOnZoneInfoDAO
    : public DataAccessObject<AddOnZoneInfoKey, std::vector<AddonZoneInfo*>, false>
{
public:
  static AddOnZoneInfoDAO& instance();

  const std::vector<AddonZoneInfo*>& get(DeleteList& del,
                                         const VendorCode& vendor,
                                         const CarrierCode& carrier,
                                         const TariffNumber& fareTariff,
                                         const AddonZone& zone,
                                         const DateTime& date,
                                         const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, AddOnZoneInfoKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b) &&
               objectKey.getValue("FARETARIFF", key._c) && objectKey.getValue("ZONE", key._d);
  }

  AddOnZoneInfoKey createKey(const AddonZoneInfo* info);

  void translateKey(const AddOnZoneInfoKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("CARRIER", key._b);
    objectKey.setValue("FARETARIFF", key._c);
    objectKey.setValue("ZONE", key._d);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<AddonZoneInfo, AddOnZoneInfoDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<AddOnZoneInfoDAO>;

  static DAOHelper<AddOnZoneInfoDAO> _helper;

  AddOnZoneInfoDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<AddOnZoneInfoKey, std::vector<AddonZoneInfo*>, false>(cacheSize, cacheType)
  {
  }

  std::vector<AddonZoneInfo*>* create(AddOnZoneInfoKey key) override;

  void destroy(AddOnZoneInfoKey key, std::vector<AddonZoneInfo*>* t) override;

  void load() override;

private:
  static AddOnZoneInfoDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: AddOnZoneInfoHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, CarrierCode, TariffNumber, AddonZone, DateTime, DateTime>
AddOnZoneInfoHistoricalKey;

class AddOnZoneInfoHistoricalDAO : public HistoricalDataAccessObject<AddOnZoneInfoHistoricalKey,
                                                                     std::vector<AddonZoneInfo*>,
                                                                     false>
{
public:
  static AddOnZoneInfoHistoricalDAO& instance();

  const std::vector<AddonZoneInfo*>& get(DeleteList& del,
                                         const VendorCode& vendor,
                                         const CarrierCode& carrier,
                                         const TariffNumber& fareTariff,
                                         const AddonZone& zone,
                                         const DateTime& date,
                                         const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, AddOnZoneInfoHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._e);
    objectKey.getValue("ENDDATE", key._f);
    return key.initialized = objectKey.getValue("VENDOR", key._a)
                             && objectKey.getValue("CARRIER", key._b)
                             && objectKey.getValue("FARETARIFF", key._c)
                             && objectKey.getValue("ZONE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    AddOnZoneInfoHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._e, key._f, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b) &&
               objectKey.getValue("FARETARIFF", key._c) && objectKey.getValue("ZONE", key._d);
  }

  void setKeyDateRange(AddOnZoneInfoHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._e, key._f, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<AddOnZoneInfoHistoricalDAO>;
  static DAOHelper<AddOnZoneInfoHistoricalDAO> _helper;

  AddOnZoneInfoHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<AddOnZoneInfoHistoricalKey, std::vector<AddonZoneInfo*>, false>(
          cacheSize, cacheType)
  {
  }

  std::vector<AddonZoneInfo*>* create(AddOnZoneInfoHistoricalKey key) override;

  void destroy(AddOnZoneInfoHistoricalKey key, std::vector<AddonZoneInfo*>* t) override;

private:
  static AddOnZoneInfoHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
