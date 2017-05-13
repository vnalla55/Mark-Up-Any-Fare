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
#include "DBAccess/AddonCombFareClassInfo.h"
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
class AddonCombFareClassInfo;
class DeleteList;

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION

class AddOnCombFareClassDAO;

// This explicit specialization must occur before 
// DummyObjectInserter instantiation in AddOnCombFareClassDAO::insertDummyObject.
template <> template <>
DataAccessObjectBase<AddOnCombFareClassKey, AddonFareClassCombMultiMap, false
                    >::DummyObjectInserter<AddonCombFareClassInfo, AddOnCombFareClassDAO
                                          >::DummyObjectInserter(std::string& flatKey,
                                                                 ObjectKey& objectKey);


class AddOnCombFareClassDAO
    : public DataAccessObject<AddOnCombFareClassKey, AddonFareClassCombMultiMap, false>
{
public:
  static AddOnCombFareClassDAO& instance();

  const AddonFareClassCombMultiMap& get(DeleteList& del,
                                        const VendorCode& vendor,
                                        const TariffNumber& fareTariff,
                                        const CarrierCode& carrier);

  bool translateKey(const ObjectKey& objectKey, AddOnCombFareClassKey& key) const override
  {
    return key.initialized = objectKey.getValue("VENDOR", key._a) &&
                             objectKey.getValue("FARETARIFF", key._b) &&
                             objectKey.getValue("CARRIER", key._c);
  }

  AddOnCombFareClassKey createKey(const AddonCombFareClassInfoKey1&);

  void translateKey(const AddOnCombFareClassKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("FARETARIFF", key._b);
    objectKey.setValue("CARRIER", key._c);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<AddonCombFareClassInfo,
                               AddOnCombFareClassDAO>(flatKey,
                                                      objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const AddonFareClassCombMultiMap* map) const override;

  virtual AddonFareClassCombMultiMap*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<AddOnCombFareClassDAO>;
  static DAOHelper<AddOnCombFareClassDAO> _helper;
  AddOnCombFareClassDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<AddOnCombFareClassKey, AddonFareClassCombMultiMap, false>(
          cacheSize, cacheType, 8)
  {
  }
  virtual void load() override;
  AddonFareClassCombMultiMap* create(AddOnCombFareClassKey key) override;
  void destroy(AddOnCombFareClassKey key, AddonFareClassCombMultiMap* t) override;

private:
  struct groupByKey;
  static AddOnCombFareClassDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class AddOnCombFareClassDAO


// Definition of this specialization requires AddOnCombFareClassDAO 
// to be fully defined.
template <> template <>
DataAccessObjectBase<AddOnCombFareClassKey, AddonFareClassCombMultiMap, false
                    >::DummyObjectInserter<AddonCombFareClassInfo, AddOnCombFareClassDAO
                                          >::DummyObjectInserter(std::string& flatKey,
                                                                 ObjectKey& objectKey)
: _success(false)
{
  AddOnCombFareClassKey key;
  AddonCombFareClassInfo* info(new AddonCombFareClassInfo);
  AddonCombFareClassInfo::dummyData(key, *info);
  AddonFareClassCombMultiMap* object(new AddonFareClassCombMultiMap);
  putInfoIntoObject(key, info, object);
  AddOnCombFareClassDAO::instance().cache().put(key, object, true);

  objectKey.tableName() = AddOnCombFareClassDAO::instance().name();
  objectKey.keyFields().clear();
  AddOnCombFareClassDAO::instance().translateKey(key, objectKey);

  KeyStream stream(0);
  stream << key;
  flatKey = stream;

  _success = true;
}

#else

typedef HashKey<VendorCode, TariffNumber, CarrierCode> AddOnCombFareClassKey;

class AddOnCombFareClassDAO
    : public DataAccessObject<AddOnCombFareClassKey, AddonFareClassCombMultiMap, false>
{
public:
  static AddOnCombFareClassDAO& instance();

  const AddonFareClassCombMultiMap& get(DeleteList& del,
                                        const VendorCode& vendor,
                                        const TariffNumber& fareTariff,
                                        const CarrierCode& carrier);

  bool translateKey(const ObjectKey& objectKey, AddOnCombFareClassKey& key) const override
  {
    return key.initialized = objectKey.getValue("VENDOR", key._a) &&
                             objectKey.getValue("FARETARIFF", key._b) &&
                             objectKey.getValue("CARRIER", key._c);
  }

  AddOnCombFareClassKey createKey(AddonCombFareClassInfo* info);

  void translateKey(const AddOnCombFareClassKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("FARETARIFF", key._b);
    objectKey.setValue("CARRIER", key._c);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<AddonCombFareClassInfo, AddOnCombFareClassDAO>(flatKey, objectKey)
        .success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const AddonFareClassCombMultiMap* map) const override;

  virtual AddonFareClassCombMultiMap*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<AddOnCombFareClassDAO>;
  static DAOHelper<AddOnCombFareClassDAO> _helper;
  AddOnCombFareClassDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<AddOnCombFareClassKey, AddonFareClassCombMultiMap, false>(
          cacheSize, cacheType, 5)
  {
  }
  virtual void load() override;
  AddonFareClassCombMultiMap* create(AddOnCombFareClassKey key) override;
  void destroy(AddOnCombFareClassKey key, AddonFareClassCombMultiMap* t) override;

private:
  struct groupByKey;
  static AddOnCombFareClassDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class AddOnCombFareClassDAO

#endif

// Historical Stuff
// ///////////////////////////////////////////////////////////////////////////////////////////////
typedef HashKey<VendorCode, TariffNumber, CarrierCode, DateTime, DateTime>
AddOnCombFareClassHistoricalKey;

class AddOnCombFareClassHistoricalDAO
    : public HistoricalDataAccessObject<AddOnCombFareClassHistoricalKey,
                                        std::vector<AddonCombFareClassInfo*> >
{
public:
  static AddOnCombFareClassHistoricalDAO& instance();

  const std::vector<AddonCombFareClassInfo*>& get(DeleteList& del,
                                                  const VendorCode& vendor,
                                                  const TariffNumber& fareTariff,
                                                  const CarrierCode& carrier,
                                                  const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, AddOnCombFareClassHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._d);
    objectKey.getValue("ENDDATE", key._e);
    return key.initialized = objectKey.getValue("VENDOR", key._a)
                             && objectKey.getValue("FARETARIFF", key._b)
                             && objectKey.getValue("CARRIER", key._c);
  }

  bool translateKey(const ObjectKey& objectKey,
                    AddOnCombFareClassHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
    return key.initialized = objectKey.getValue("VENDOR", key._a) &&
                             objectKey.getValue("FARETARIFF", key._b) &&
                             objectKey.getValue("CARRIER", key._c);
  }

  void
  setKeyDateRange(AddOnCombFareClassHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData*
  compress(const std::vector<AddonCombFareClassInfo*>* vect) const override;

  virtual std::vector<AddonCombFareClassInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<AddOnCombFareClassHistoricalDAO>;
  static DAOHelper<AddOnCombFareClassHistoricalDAO> _helper;
  AddOnCombFareClassHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<AddOnCombFareClassHistoricalKey,
#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
                                 std::vector<AddonCombFareClassInfo*> >(cacheSize, cacheType, 7)
#else
                                 std::vector<AddonCombFareClassInfo*> >(cacheSize, cacheType, 6)
#endif
  {
  }
  std::vector<AddonCombFareClassInfo*>* create(AddOnCombFareClassHistoricalKey key) override;
  void
  destroy(AddOnCombFareClassHistoricalKey key, std::vector<AddonCombFareClassInfo*>* t) override;

private:
  static AddOnCombFareClassHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class AddOnCombFareClassHistoricalDAO
} // namespace tse
