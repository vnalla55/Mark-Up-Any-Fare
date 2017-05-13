//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
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
class CarrierApplicationInfo;
class DeleteList;

typedef HashKey<VendorCode, int> CarrierApplicationKey;

class CarrierApplicationDAO
    : public DataAccessObject<CarrierApplicationKey, std::vector<CarrierApplicationInfo*> >
{
public:
  static CarrierApplicationDAO& instance();
  const std::vector<CarrierApplicationInfo*>&
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, CarrierApplicationKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  CarrierApplicationKey createKey(const CarrierApplicationInfo* info);

  void translateKey(const CarrierApplicationKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<CarrierApplicationInfo, CarrierApplicationDAO>(flatKey, objectKey)
        .success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<CarrierApplicationDAO>;
  static DAOHelper<CarrierApplicationDAO> _helper;
  CarrierApplicationDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<CarrierApplicationKey, std::vector<CarrierApplicationInfo*> >(
          cacheSize, cacheType, 2)
  {
  }
  std::vector<CarrierApplicationInfo*>* create(CarrierApplicationKey key) override;
  void destroy(CarrierApplicationKey key, std::vector<CarrierApplicationInfo*>* t) override;
  void load() override;

private:
  static CarrierApplicationDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class CarrierApplicationDAO

// --------------------------------------------------
// Historical DAO: CarrierApplicationHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> CarrierApplicationHistoricalKey;

class CarrierApplicationHistoricalDAO
    : public HistoricalDataAccessObject<CarrierApplicationHistoricalKey,
                                        std::vector<CarrierApplicationInfo*> >
{
public:
  static CarrierApplicationHistoricalDAO& instance();

  const std::vector<CarrierApplicationInfo*>&
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, CarrierApplicationHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._c);
    objectKey.getValue("ENDDATE", key._d);
    return key.initialized = objectKey.getValue("VENDOR", key._a)
                             && objectKey.getValue("ITEMNO", key._b);
  }

  bool translateKey(const ObjectKey& objectKey,
                    CarrierApplicationHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void
  setKeyDateRange(CarrierApplicationHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<CarrierApplicationHistoricalDAO>;
  static DAOHelper<CarrierApplicationHistoricalDAO> _helper;
  CarrierApplicationHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<CarrierApplicationHistoricalKey,
                                 std::vector<CarrierApplicationInfo*> >(cacheSize, cacheType, 2)
  {
  }
  std::vector<CarrierApplicationInfo*>* create(CarrierApplicationHistoricalKey key) override;
  void
  destroy(CarrierApplicationHistoricalKey key, std::vector<CarrierApplicationInfo*>* t) override;

private:
  static CarrierApplicationHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class CarrierApplicationHistoricalDAO
} // namespace tse

