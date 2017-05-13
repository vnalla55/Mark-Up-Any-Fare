//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
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
class OptionalServicesInfo;

typedef HashKey<VendorCode, CarrierCode, Indicator> OptionalServicesKey;
typedef HashKey<VendorCode, CarrierCode, Indicator, DateTime, DateTime>
OptionalServicesHistoricalKey;

class OptionalServicesDAO
    : public DataAccessObject<OptionalServicesKey, std::vector<OptionalServicesInfo*> >
{
public:
  friend class DAOHelper<OptionalServicesDAO>;

  static OptionalServicesDAO& instance();

  const std::vector<OptionalServicesInfo*>& get(DeleteList& del,
                                                const VendorCode& vendor,
                                                const CarrierCode& carrier,
                                                const ServiceTypeCode& serviceTypeCode,
                                                const ServiceSubTypeCode& serviceSubTypeCode,
                                                Indicator fltTktMerchInd,
                                                const DateTime& date,
                                                const DateTime& ticketDate);

  OptionalServicesKey createKey(OptionalServicesInfo* info);

  bool translateKey(const ObjectKey& objectKey, OptionalServicesKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b);
  }

  void translateKey(const OptionalServicesKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("CARRIER", key._b);
  }

  virtual size_t invalidate(const ObjectKey& objectKey) override;

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<OptionalServicesInfo, OptionalServicesDAO>(flatKey, objectKey)
        .success();
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  virtual sfc::CompressedData*
  compress(const std::vector<OptionalServicesInfo*>* vect) const override;

  virtual std::vector<OptionalServicesInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

private:
  OptionalServicesDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<OptionalServicesKey, std::vector<OptionalServicesInfo*> >(
          cacheSize, cacheType, 8)
  {
  }

  std::vector<OptionalServicesInfo*>* create(OptionalServicesKey key) override;
  void destroy(OptionalServicesKey key, std::vector<OptionalServicesInfo*>* recs) override;
  void load() override;

  static std::string _name;
  static std::string _cacheClass;
  static DAOHelper<OptionalServicesDAO> _helper;
  static OptionalServicesDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

class OptionalServicesHistoricalDAO
    : public HistoricalDataAccessObject<OptionalServicesHistoricalKey,
                                        std::vector<OptionalServicesInfo*> >
{
public:
  friend class DAOHelper<OptionalServicesHistoricalDAO>;

  static OptionalServicesHistoricalDAO& instance();

  const std::vector<OptionalServicesInfo*>& get(DeleteList& del,
                                                const VendorCode& vendor,
                                                const CarrierCode& carrier,
                                                const ServiceTypeCode& serviceTypeCode,
                                                const ServiceSubTypeCode& serviceSubTypeCode,
                                                Indicator fltTktMerchInd,
                                                const DateTime& date,
                                                const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, OptionalServicesHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b);
  }

  bool translateKey(const ObjectKey& objectKey,
                    OptionalServicesHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);

    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b);
  }

  virtual size_t invalidate(const ObjectKey& objectKey) override;

  void setKeyDateRange(OptionalServicesHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  virtual sfc::CompressedData*
  compress(const std::vector<OptionalServicesInfo*>* vect) const override;

  virtual std::vector<OptionalServicesInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

private:
  OptionalServicesHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<OptionalServicesHistoricalKey,
                                 std::vector<OptionalServicesInfo*> >(cacheSize, cacheType, 8)
  {
  }
  std::vector<OptionalServicesInfo*>* create(OptionalServicesHistoricalKey key) override;
  void destroy(OptionalServicesHistoricalKey key, std::vector<OptionalServicesInfo*>* t) override;

  static std::string _name;
  static std::string _cacheClass;
  static DAOHelper<OptionalServicesHistoricalDAO> _helper;
  static OptionalServicesHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // tse
