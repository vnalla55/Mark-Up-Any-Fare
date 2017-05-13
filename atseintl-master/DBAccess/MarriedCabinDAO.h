//------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product
// of Sabre Inc.  Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly
// prohibited.
//------------------------------------------------------------------------------
//
#pragma once

#include "Common/Logger.h"
#include "Common/TseCodeTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/HistoricalDataAccessObject.h"
#include "DBAccess/MarriedCabin.h"

namespace tse
{
typedef HashKey<CarrierCode, BookingCode> MarriedCabinKey;

class MarriedCabinDAO : public DataAccessObject<MarriedCabinKey, std::vector<MarriedCabin*>>
{
public:
  static MarriedCabinDAO& instance();

  const std::vector<MarriedCabin*>& get(DeleteList& del,
                                        const CarrierCode& carrier,
                                        const BookingCode& premiumCabin,
                                        const DateTime& versionDate,
                                        const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objKey, MarriedCabinKey& key) const override
  {
    return key.initialized =
               objKey.getValue("CARRIER", key._a) && objKey.getValue("PREMIUMCABIN", key._b);
  }

  MarriedCabinKey createKey(MarriedCabin* info);

  void translateKey(const MarriedCabinKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CARRIER", key._a);
    objectKey.setValue("PREMIUMCABIN", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<MarriedCabin, MarriedCabinDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;

  friend class DAOHelper<MarriedCabinDAO>;

  static DAOHelper<MarriedCabinDAO> _helper;

  MarriedCabinDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<MarriedCabinKey, std::vector<MarriedCabin*> >(cacheSize, cacheType)
  {
  }

  void load() override;

  std::vector<MarriedCabin*>* create(MarriedCabinKey key) override;

  void destroy(MarriedCabinKey key, std::vector<MarriedCabin*>* t) override;

private:
  static MarriedCabinDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: MarriedCabinHistoricalDAO
// --------------------------------------------------
class MarriedCabinHistoricalDAO
    : public HistoricalDataAccessObject<MarriedCabinKey, std::vector<MarriedCabin*> >
{
public:
  static MarriedCabinHistoricalDAO& instance();

  const std::vector<MarriedCabin*>& get(DeleteList& del,
                                        const CarrierCode& carrier,
                                        const BookingCode& premiumCabin,
                                        const DateTime& versionDate,
                                        const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objKey, MarriedCabinKey& key) const override
  {
    return key.initialized =
               objKey.getValue("CARRIER", key._a) && objKey.getValue("PREMIUMCABIN", key._b);
  }

protected:
  static std::string _name;

  friend class DAOHelper<MarriedCabinHistoricalDAO>;

  static DAOHelper<MarriedCabinHistoricalDAO> _helper;

  MarriedCabinHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<MarriedCabinKey, std::vector<MarriedCabin*> >(cacheSize, cacheType)
  {
  }

  std::vector<MarriedCabin*>* create(MarriedCabinKey key) override;

  void destroy(MarriedCabinKey key, std::vector<MarriedCabin*>* t) override;

  virtual void load() override;

private:
  struct groupByKey;
  static MarriedCabinHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
}
