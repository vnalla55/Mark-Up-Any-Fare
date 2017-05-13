//-------------------------------------------------------------------------------
// Copyright 2012, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
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
class USDotCarrier;

typedef HashKey<CarrierCode> USDotCarrierKey;

class USDotCarrierDAO : public DataAccessObject<USDotCarrierKey, std::vector<USDotCarrier*> >
{
public:
  static USDotCarrierDAO& instance();

  bool get(const CarrierCode& carrier);

  bool translateKey(const ObjectKey& objectKey, USDotCarrierKey& key) const override
  {
    return key.initialized = objectKey.getValue("CARRIER", key._a);
  }

  USDotCarrierKey createKey(const USDotCarrier* info);

  void translateKey(const USDotCarrierKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CARRIER", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<USDotCarrier, USDotCarrierDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<USDotCarrierDAO>;
  static DAOHelper<USDotCarrierDAO> _helper;

  USDotCarrierDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<USDotCarrierKey, std::vector<USDotCarrier*> >(cacheSize, cacheType)
  {
  }

  void load() override;
  std::vector<USDotCarrier*>* create(USDotCarrierKey key) override;
  void destroy(USDotCarrierKey key, std::vector<USDotCarrier*>* t) override;

private:
  static USDotCarrierDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: USDotCarrierHistoricalDAO
// --------------------------------------------------
typedef HashKey<CarrierCode> USDotCarrierHistoricalKey;

class USDotCarrierHistoricalDAO
    : public HistoricalDataAccessObject<USDotCarrierHistoricalKey, std::vector<USDotCarrier*> >
{
public:
  static USDotCarrierHistoricalDAO& instance();
  bool get(const CarrierCode& carrier, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, USDotCarrierHistoricalKey& key) const override
  {
    return key.initialized = objectKey.getValue("CARRIER", key._a);
  }

  bool translateKey(const ObjectKey& objectKey,
                    USDotCarrierHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    return key.initialized = objectKey.getValue("CARRIER", key._a);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<USDotCarrierHistoricalDAO>;
  static DAOHelper<USDotCarrierHistoricalDAO> _helper;

  USDotCarrierHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<USDotCarrierHistoricalKey, std::vector<USDotCarrier*> >(cacheSize,
                                                                                         cacheType)
  {
  }

  std::vector<USDotCarrier*>* create(USDotCarrierHistoricalKey key) override;
  void destroy(USDotCarrierHistoricalKey key, std::vector<USDotCarrier*>* t) override;

private:
  static USDotCarrierHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

} // namespace tse

