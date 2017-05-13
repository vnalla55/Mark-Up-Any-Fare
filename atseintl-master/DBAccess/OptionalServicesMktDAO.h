//-------------------------------------------------------------------------------
// Copyright 2011, Sabre Inc.  All rights reserved.
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

typedef HashKey<VendorCode, CarrierCode, LocCode, LocCode> OptionalServicesMktKey;
typedef HashKey<VendorCode, CarrierCode, LocCode, LocCode, DateTime, DateTime>
OptionalServicesMktHistoricalKey;

class OptionalServicesMktDAO
    : public DataAccessObject<OptionalServicesMktKey, std::vector<OptionalServicesInfo*> >
{
public:
  friend class DAOHelper<OptionalServicesMktDAO>;

  static OptionalServicesMktDAO& instance();

  const std::vector<OptionalServicesInfo*>& get(DeleteList& del,
                                                const VendorCode& vendor,
                                                const CarrierCode& carrier,
                                                const LocCode& loc1,
                                                const LocCode& loc2,
                                                const ServiceTypeCode& serviceTypeCode,
                                                const ServiceSubTypeCode& serviceSubTypeCode,
                                                Indicator fltTktMerchInd,
                                                const DateTime& date,
                                                const DateTime& ticketDate);

  OptionalServicesMktKey createKey(OptionalServicesInfo* info);

  bool translateKey(const ObjectKey& objectKey, OptionalServicesMktKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b);
  }

  void translateKey(const OptionalServicesMktKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("CARRIER", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<OptionalServicesInfo, OptionalServicesMktDAO>(flatKey, objectKey)
        .success();
  }

  virtual size_t invalidate(const ObjectKey& objectKey) override;

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

private:
  OptionalServicesMktDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<OptionalServicesMktKey, std::vector<OptionalServicesInfo*> >(
          cacheSize, cacheType, 5)
  {
  }

  std::vector<OptionalServicesInfo*>* create(OptionalServicesMktKey key) override;
  void destroy(OptionalServicesMktKey key, std::vector<OptionalServicesInfo*>* recs) override;
  void load() override;

  static std::string _name;
  static std::string _cacheClass;
  static DAOHelper<OptionalServicesMktDAO> _helper;
  static OptionalServicesMktDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

class OptionalServicesMktHistoricalDAO
    : public HistoricalDataAccessObject<OptionalServicesMktHistoricalKey,
                                        std::vector<OptionalServicesInfo*> >
{
public:
  friend class DAOHelper<OptionalServicesMktHistoricalDAO>;

  static OptionalServicesMktHistoricalDAO& instance();

  const std::vector<OptionalServicesInfo*>& get(DeleteList& del,
                                                const VendorCode& vendor,
                                                const CarrierCode& carrier,
                                                const LocCode& loc1,
                                                const LocCode& loc2,
                                                const ServiceTypeCode& serviceTypeCode,
                                                const ServiceSubTypeCode& serviceSubTypeCode,
                                                Indicator fltTktMerchInd,
                                                const DateTime& date,
                                                const DateTime& ticketDate);

  bool
  translateKey(const ObjectKey& objectKey, OptionalServicesMktHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b) &&
               objectKey.getValue("STARTDATE", key._e) && objectKey.getValue("ENDDATE", key._f);
  }

  bool translateKey(const ObjectKey& objectKey,
                    OptionalServicesMktHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._e, key._f, _cacheBy);

    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b);
  }

  void
  setKeyDateRange(OptionalServicesMktHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._e, key._f, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

private:
  OptionalServicesMktHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<OptionalServicesMktHistoricalKey,
                                 std::vector<OptionalServicesInfo*> >(cacheSize, cacheType, 5)
  {
  }
  std::vector<OptionalServicesInfo*>* create(OptionalServicesMktHistoricalKey key) override;
  void
  destroy(OptionalServicesMktHistoricalKey key, std::vector<OptionalServicesInfo*>* t) override;

  static std::string _name;
  static std::string _cacheClass;
  static DAOHelper<OptionalServicesMktHistoricalDAO> _helper;
  static OptionalServicesMktHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // tse
