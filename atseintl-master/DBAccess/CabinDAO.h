//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/HistoricalDataAccessObject.h"

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class Cabin;
class DeleteList;

class CabinDAO : public DataAccessObject<CarrierKey, std::vector<Cabin*>, false>
{
public:
  static CabinDAO& instance();

  const Cabin* get(DeleteList& del,
                   const CarrierCode& carrier,
                   const BookingCode& classOfService,
                   const DateTime& date,
                   const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, CarrierKey& key) const override
  {
    return key.initialized = objectKey.getValue("CARRIER", key._a);
  }

  CarrierKey createKey(Cabin* info);

  void translateKey(const CarrierKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CARRIER", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<Cabin, CabinDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<CabinDAO>;

  static DAOHelper<CabinDAO> _helper;

  CabinDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<CarrierKey, std::vector<Cabin*>, false>(cacheSize, cacheType)
  {
  }

  std::vector<Cabin*>* create(CarrierKey key) override;
  void destroy(CarrierKey key, std::vector<Cabin*>* recs) override;

  virtual void load() override;

private:
  struct isEffective;
  static CabinDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: CabinHistoricalDAO
// --------------------------------------------------
class CabinHistoricalDAO : public HistoricalDataAccessObject<CarrierKey, std::vector<Cabin*>, false>
{
public:
  static CabinHistoricalDAO& instance();

  const Cabin* get(DeleteList& del,
                   const CarrierCode& carrier,
                   const BookingCode& classOfService,
                   const DateTime& date,
                   const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, CarrierKey& key) const override
  {
    return key.initialized = objectKey.getValue("CARRIER", key._a);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  CarrierKey createKey(Cabin* info,
                       const DateTime& startDate = DateTime::emptyDate(),
                       const DateTime& endDate = DateTime::emptyDate());

  void translateKey(const CarrierKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CARRIER", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<Cabin, CabinHistoricalDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<CabinHistoricalDAO>;

  static DAOHelper<CabinHistoricalDAO> _helper;

  CabinHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<CarrierKey, std::vector<Cabin*>, false>(cacheSize, cacheType)
  {
  }

  std::vector<Cabin*>* create(CarrierKey key) override;
  void destroy(CarrierKey key, std::vector<Cabin*>* recs) override;

  virtual void load() override;

private:
  struct isEffective;
  struct groupByKey;
  static CabinHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse

