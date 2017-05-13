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
#include "DBAccess/AddonZoneInfo.h"
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

typedef HashKey<VendorCode, CarrierCode, LocCode> AddOnZoneKey;

class AddOnZoneDAO : public DataAccessObject<AddOnZoneKey, std::vector<AddonZoneInfo*>, false>
{
public:
  static AddOnZoneDAO& instance();

  const std::vector<AddonZoneInfo*>& get(DeleteList& del,
                                         const VendorCode& vendor,
                                         const CarrierCode& carrier,
                                         const LocCode& market,
                                         const DateTime& date,
                                         const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, AddOnZoneKey& key) const override
  {
    return key.initialized = objectKey.getValue("VENDOR", key._a) &&
                             objectKey.getValue("CARRIER", key._b) &&
                             objectKey.getValue("LOC", key._c);
  }

  AddOnZoneKey createKey(AddonZoneInfo* info);

  void translateKey(const AddOnZoneKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("CARRIER", key._b);
    objectKey.setValue("LOC", key._c);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<AddonZoneInfo, AddOnZoneDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<AddonZoneInfo*>* vect) const override;

  virtual std::vector<AddonZoneInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<AddOnZoneDAO>;

  static DAOHelper<AddOnZoneDAO> _helper;

  AddOnZoneDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<AddOnZoneKey, std::vector<AddonZoneInfo*>, false>(cacheSize, cacheType)
  {
  }

  virtual void load() override;

  std::vector<AddonZoneInfo*>* create(AddOnZoneKey key) override;

  void destroy(AddOnZoneKey key, std::vector<AddonZoneInfo*>* t) override;

private:
  struct isEffective;
  struct groupByKey;
  static AddOnZoneDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class AddOnZoneDAO

// Historical Stuff ///////////////////////////////////////////////////////////////////////////////
typedef HashKey<VendorCode, CarrierCode, LocCode, DateTime, DateTime> AddOnZoneHistoricalKey;

class AddOnZoneHistoricalDAO
    : public HistoricalDataAccessObject<AddOnZoneHistoricalKey, std::vector<AddonZoneInfo*> >
{
public:
  static AddOnZoneHistoricalDAO& instance();

  const std::vector<AddonZoneInfo*>& get(DeleteList& del,
                                         const VendorCode& vendor,
                                         const CarrierCode& carrier,
                                         const LocCode& market,
                                         const DateTime& date,
                                         const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, AddOnZoneHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._d);
    objectKey.getValue("ENDDATE", key._e);
    return key.initialized = objectKey.getValue("VENDOR", key._a)
                             && objectKey.getValue("CARRIER", key._b)
                             && objectKey.getValue("LOC", key._c);
  }

  bool translateKey(const ObjectKey& objectKey,
                    AddOnZoneHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
    return key.initialized = objectKey.getValue("VENDOR", key._a) &&
                             objectKey.getValue("CARRIER", key._b) &&
                             objectKey.getValue("LOC", key._c);
  }

  void setKeyDateRange(AddOnZoneHistoricalKey& key, const DateTime ticketDate) const override
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
  friend class DAOHelper<AddOnZoneHistoricalDAO>;
  static DAOHelper<AddOnZoneHistoricalDAO> _helper;

  AddOnZoneHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<AddOnZoneHistoricalKey, std::vector<AddonZoneInfo*> >(cacheSize,
                                                                                       cacheType)
  {
  }

  std::vector<AddonZoneInfo*>* create(AddOnZoneHistoricalKey key) override;

  void destroy(AddOnZoneHistoricalKey key, std::vector<AddonZoneInfo*>* t) override;

private:
  static AddOnZoneHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class AddOnZoneHistoricalDAO

} // namespace tse
