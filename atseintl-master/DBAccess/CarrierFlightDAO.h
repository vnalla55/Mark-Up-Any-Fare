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
class CarrierFlight;
class DeleteList;

typedef HashKey<VendorCode, int> CarrierFlightKey;

class CarrierFlightDAO : public DataAccessObject<CarrierFlightKey, std::vector<CarrierFlight*> >
{
public:
  static CarrierFlightDAO& instance();
  const CarrierFlight*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, CarrierFlightKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  CarrierFlightKey createKey(CarrierFlight* info);

  void translateKey(const CarrierFlightKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<CarrierFlight, CarrierFlightDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<CarrierFlight*>* vect) const override;

  virtual std::vector<CarrierFlight*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<CarrierFlightDAO>;
  static DAOHelper<CarrierFlightDAO> _helper;
  CarrierFlightDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<CarrierFlightKey, std::vector<CarrierFlight*> >(cacheSize, cacheType)
  {
  }
  void load() override;
  std::vector<CarrierFlight*>* create(CarrierFlightKey key) override;
  void destroy(CarrierFlightKey key, std::vector<CarrierFlight*>* t) override;

private:
  static CarrierFlightDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: CarrierFlightHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> CarrierFlightHistoricalKey;

class CarrierFlightHistoricalDAO
    : public HistoricalDataAccessObject<CarrierFlightHistoricalKey, std::vector<CarrierFlight*> >
{
public:
  static CarrierFlightHistoricalDAO& instance();
  const CarrierFlight*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, CarrierFlightHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._c);
    objectKey.getValue("ENDDATE", key._d);
    return key.initialized = objectKey.getValue("VENDOR", key._a)
                             && objectKey.getValue("ITEMNO", key._b);
  }

  bool translateKey(const ObjectKey& objectKey,
                    CarrierFlightHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void setKeyDateRange(CarrierFlightHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<CarrierFlight*>* vect) const override;

  virtual std::vector<CarrierFlight*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<CarrierFlightHistoricalDAO>;
  static DAOHelper<CarrierFlightHistoricalDAO> _helper;
  CarrierFlightHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<CarrierFlightHistoricalKey, std::vector<CarrierFlight*> >(
          cacheSize, cacheType)
  {
  }
  std::vector<CarrierFlight*>* create(CarrierFlightHistoricalKey key) override;
  void destroy(CarrierFlightHistoricalKey key, std::vector<CarrierFlight*>* t) override;

private:
  static CarrierFlightHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse

