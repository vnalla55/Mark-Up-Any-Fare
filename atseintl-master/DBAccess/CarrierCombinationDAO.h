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
class CarrierCombination;
class DeleteList;

typedef HashKey<VendorCode, int> CarrierCombinationKey;

class CarrierCombinationDAO
    : public DataAccessObject<CarrierCombinationKey, std::vector<CarrierCombination*> >
{
public:
  static CarrierCombinationDAO& instance();
  const std::vector<CarrierCombination*>&
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, CarrierCombinationKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  CarrierCombinationKey createKey(CarrierCombination* info);

  void translateKey(const CarrierCombinationKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<CarrierCombination, CarrierCombinationDAO>(flatKey, objectKey)
        .success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<CarrierCombinationDAO>;
  static DAOHelper<CarrierCombinationDAO> _helper;
  CarrierCombinationDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<CarrierCombinationKey, std::vector<CarrierCombination*> >(cacheSize,
                                                                                 cacheType)
  {
  }
  void load() override;
  std::vector<CarrierCombination*>* create(CarrierCombinationKey key) override;
  void destroy(CarrierCombinationKey key, std::vector<CarrierCombination*>* t) override;

private:
  static CarrierCombinationDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: CarrierCombinationHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> CarrierCombinationHistoricalKey;

class CarrierCombinationHistoricalDAO
    : public HistoricalDataAccessObject<CarrierCombinationHistoricalKey,
                                        std::vector<CarrierCombination*> >
{
public:
  static CarrierCombinationHistoricalDAO& instance();
  const std::vector<CarrierCombination*>&
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, CarrierCombinationHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._c);
    objectKey.getValue("ENDDATE", key._d);
    return key.initialized = objectKey.getValue("VENDOR", key._a)
                             && objectKey.getValue("ITEMNO", key._b);
  }

  bool translateKey(const ObjectKey& objectKey,
                    CarrierCombinationHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void
  setKeyDateRange(CarrierCombinationHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<CarrierCombinationHistoricalDAO>;
  static DAOHelper<CarrierCombinationHistoricalDAO> _helper;
  CarrierCombinationHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<CarrierCombinationHistoricalKey,
                                 std::vector<CarrierCombination*> >(cacheSize, cacheType)
  {
  }
  std::vector<CarrierCombination*>* create(CarrierCombinationHistoricalKey key) override;
  void destroy(CarrierCombinationHistoricalKey key, std::vector<CarrierCombination*>* t) override;

private:
  static CarrierCombinationHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse

