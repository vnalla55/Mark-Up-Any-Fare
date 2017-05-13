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
#include "DBAccess/Loc.h"

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

typedef HashKey<LocCode, VendorCode, CarrierCode> AddonZoneSITAKey;

class AddonZoneSITADAO : public DataAccessObject<AddonZoneSITAKey, std::vector<AddonZoneInfo*> >
{
public:
  static AddonZoneSITADAO& instance();

  const std::vector<AddonZoneInfo*>& get(DeleteList& del,
                                         const LocCode& loc,
                                         const VendorCode& vendor,
                                         const CarrierCode& carrier,
                                         const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, AddonZoneSITAKey& key) const override
  {
    return key.initialized = objectKey.getValue("LOC", key._a) &&
                             objectKey.getValue("VENDOR", key._b) &&
                             objectKey.getValue("CARRIER", key._c);
  }

  AddonZoneSITAKey createKey(const AddonZoneInfo* info);

  void translateKey(const AddonZoneSITAKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("LOC", key._a);
    objectKey.setValue("VENDOR", key._b);
    objectKey.setValue("CARRIER", key._c);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<AddonZoneInfo, AddonZoneSITADAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<AddonZoneInfo*>* vect) const override;

  virtual std::vector<AddonZoneInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<AddonZoneSITADAO>;
  static DAOHelper<AddonZoneSITADAO> _helper;
  AddonZoneSITADAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<AddonZoneSITAKey, std::vector<AddonZoneInfo*> >(cacheSize, cacheType)
  {
  }
  std::vector<AddonZoneInfo*>* create(AddonZoneSITAKey key) override;
  void destroy(AddonZoneSITAKey key, std::vector<AddonZoneInfo*>* t) override;
  void load() override;

private:
  static AddonZoneSITADAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class AddonZoneSITADAO

// Historical Stuff ///////////////////////////////////////////////////////////////////////
typedef HashKey<LocCode, VendorCode, CarrierCode, DateTime, DateTime> AddonZoneSITAHistoricalKey;

class AddonZoneSITAHistoricalDAO
    : public HistoricalDataAccessObject<AddonZoneSITAHistoricalKey, std::vector<AddonZoneInfo*> >
{
public:
  static AddonZoneSITAHistoricalDAO& instance();

  const std::vector<AddonZoneInfo*>& get(DeleteList& del,
                                         const LocCode& loc,
                                         const VendorCode& vendor,
                                         const CarrierCode& carrier,
                                         const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, AddonZoneSITAHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._d);
    objectKey.getValue("ENDDATE", key._e);
    return key.initialized = objectKey.getValue("LOC", key._a)
                             && objectKey.getValue("VENDOR", key._b)
                             && objectKey.getValue("CARRIER", key._c);
  }

  bool translateKey(const ObjectKey& objectKey,
                    AddonZoneSITAHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
    return key.initialized = objectKey.getValue("LOC", key._a) &&
                             objectKey.getValue("VENDOR", key._b) &&
                             objectKey.getValue("CARRIER", key._c);
  }

  void setKeyDateRange(AddonZoneSITAHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<AddonZoneInfo*>* vect) const override;

  virtual std::vector<AddonZoneInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<AddonZoneSITAHistoricalDAO>;
  static DAOHelper<AddonZoneSITAHistoricalDAO> _helper;
  AddonZoneSITAHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<AddonZoneSITAHistoricalKey, std::vector<AddonZoneInfo*> >(
          cacheSize, cacheType)
  {
  }
  std::vector<AddonZoneInfo*>* create(AddonZoneSITAHistoricalKey key) override;
  void destroy(AddonZoneSITAHistoricalKey key, std::vector<AddonZoneInfo*>* t) override;

private:
  static AddonZoneSITAHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class AddonZoneSITAHistoricalDAO
} // namespace tse
