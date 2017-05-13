//-------------------------------------------------------------------------------
// Copyright 2004, 2005, Sabre Inc.  All rights reserved.
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

namespace tse
{
class MinFareDefaultLogic;
class DeleteList;

class MinFareDefaultLogicDAO
    : public DataAccessObject<CarrierKey, std::vector<MinFareDefaultLogic*>, false>
{
public:
  static MinFareDefaultLogicDAO& instance();

  const std::vector<MinFareDefaultLogic*>& get(DeleteList& del,
                                               const VendorCode& vendor,
                                               const CarrierCode& governingCarrier,
                                               const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, CarrierKey& key) const override
  {
    return key.initialized = objectKey.getValue("GOVERNINGCARRIER", key._a);
  }

  CarrierKey createKey(MinFareDefaultLogic* info);

  void translateKey(const CarrierKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("GOVERNINGCARRIER", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<MinFareDefaultLogic, MinFareDefaultLogicDAO>(flatKey, objectKey)
        .success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<MinFareDefaultLogicDAO>;
  static DAOHelper<MinFareDefaultLogicDAO> _helper;
  MinFareDefaultLogicDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<CarrierKey, std::vector<MinFareDefaultLogic*>, false>(
          cacheSize, cacheType, 2)
  {
  }
  std::vector<MinFareDefaultLogic*>* create(CarrierKey key) override;
  virtual void load() override;
  void destroy(CarrierKey key, std::vector<MinFareDefaultLogic*>* t) override;

private:
  struct IsNotCurrentAndVendorMatchG;
  static MinFareDefaultLogicDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

typedef HashKey<CarrierCode, VendorCode, DateTime, DateTime> MinFareDefaultLogicHistoricalKey;

class MinFareDefaultLogicHistoricalDAO
    : public HistoricalDataAccessObject<MinFareDefaultLogicHistoricalKey,
                                        std::vector<MinFareDefaultLogic*> >
{
public:
  static MinFareDefaultLogicHistoricalDAO& instance();

  const std::vector<MinFareDefaultLogic*>& get(DeleteList& del,
                                               const VendorCode& vendor,
                                               const CarrierCode& governingCarrier,
                                               const DateTime& ticketDate);

  bool
  translateKey(const ObjectKey& objectKey, MinFareDefaultLogicHistoricalKey& key) const override
  {
    return key.initialized = objectKey.getValue("GOVERNINGCARRIER", key._a) &&
                             objectKey.getValue("VENDOR", key._b) &&
                             objectKey.getValue("STARTDATE", key._c) &&
                             objectKey.getValue("ENDDATE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    MinFareDefaultLogicHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized = objectKey.getValue("GOVERNINGCARRIER", key._a) &&
                             objectKey.getValue("VENDOR", key._b);
  }

  void
  setKeyDateRange(MinFareDefaultLogicHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<MinFareDefaultLogicHistoricalDAO>;
  static DAOHelper<MinFareDefaultLogicHistoricalDAO> _helper;
  MinFareDefaultLogicHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<MinFareDefaultLogicHistoricalKey,
                                 std::vector<MinFareDefaultLogic*> >(cacheSize, cacheType)
  {
  }
  std::vector<MinFareDefaultLogic*>* create(MinFareDefaultLogicHistoricalKey key) override;
  void destroy(MinFareDefaultLogicHistoricalKey key, std::vector<MinFareDefaultLogic*>* t) override;

  virtual void load() override;

private:
  struct groupByKey;
  static MinFareDefaultLogicHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
