//-------------------------------------------------------------------------------
// Copyright 2014, Sabre Inc.  All rights reserved.
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
class CTACarrier;

typedef HashKey<CarrierCode> CTACarrierKey;

class CTACarrierDAO : public DataAccessObject<CTACarrierKey, std::vector<CTACarrier*> >
{
public:
  static CTACarrierDAO& instance();

  bool get(const CarrierCode& carrier);

  bool translateKey(const ObjectKey& objectKey, CTACarrierKey& key) const override
  {
    return key.initialized = objectKey.getValue("CARRIER", key._a);
  }

  CTACarrierKey createKey(const CTACarrier* info);

  void translateKey(const CTACarrierKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CARRIER", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<CTACarrier, CTACarrierDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<CTACarrierDAO>;
  static DAOHelper<CTACarrierDAO> _helper;

  CTACarrierDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<CTACarrierKey, std::vector<CTACarrier*> >(cacheSize, cacheType)
  {
  }

  void load() override;
  std::vector<CTACarrier*>* create(CTACarrierKey key) override;
  void destroy(CTACarrierKey key, std::vector<CTACarrier*>* t) override;

private:
  static CTACarrierDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

typedef HashKey<CarrierCode> CTACarrierHistoricalKey;

class CTACarrierHistoricalDAO
    : public HistoricalDataAccessObject<CTACarrierHistoricalKey, std::vector<CTACarrier*> >
{
public:
  static CTACarrierHistoricalDAO& instance();
  bool get(const CarrierCode& carrier, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, CTACarrierHistoricalKey& key) const override
  {
    return key.initialized = objectKey.getValue("CARRIER", key._a);
  }

  bool translateKey(const ObjectKey& objectKey,
                    CTACarrierHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    return key.initialized = objectKey.getValue("CARRIER", key._a);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<CTACarrierHistoricalDAO>;
  static DAOHelper<CTACarrierHistoricalDAO> _helper;

  CTACarrierHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<CTACarrierHistoricalKey, std::vector<CTACarrier*> >(cacheSize,
                                                                                     cacheType)
  {
  }

  std::vector<CTACarrier*>* create(CTACarrierHistoricalKey key) override;
  void destroy(CTACarrierHistoricalKey key, std::vector<CTACarrier*>* t) override;

private:
  static CTACarrierHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

} // namespace tse

