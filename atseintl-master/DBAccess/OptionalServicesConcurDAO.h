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
class OptionalServicesConcur;

typedef HashKey<VendorCode, CarrierCode, ServiceTypeCode> OptionalServicesConcurKey;
typedef HashKey<VendorCode, CarrierCode, ServiceTypeCode, DateTime, DateTime>
OptionalServicesConcurHistoricalKey;

class OptionalServicesConcurDAO
    : public DataAccessObject<OptionalServicesConcurKey, std::vector<OptionalServicesConcur*> >
{
public:
  friend class DAOHelper<OptionalServicesConcurDAO>;

  static OptionalServicesConcurDAO& instance();

  const std::vector<OptionalServicesConcur*>& get(DeleteList& del,
                                                  const VendorCode& vendor,
                                                  const CarrierCode& carrier,
                                                  const ServiceTypeCode& serviceTypeCode,
                                                  const DateTime& date,
                                                  const DateTime& ticketDate);

  OptionalServicesConcurKey createKey(OptionalServicesConcur* concur);

  bool translateKey(const ObjectKey& objectKey, OptionalServicesConcurKey& key) const override
  {
    objectKey.getValue("SERVICETYPECODE", key._c);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b);
  }

  void translateKey(const OptionalServicesConcurKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("CARRIER", key._b);
    objectKey.setValue("SERVICETYPECODE", key._c);
  }

  virtual size_t invalidate(const ObjectKey& objectKey) override;

  const std::string& cacheClass() override { return _cacheClass; }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<OptionalServicesConcur, OptionalServicesConcurDAO>(
               flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

private:
  OptionalServicesConcurDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<OptionalServicesConcurKey, std::vector<OptionalServicesConcur*> >(cacheSize,
                                                                                         cacheType)
  {
  }

  std::vector<OptionalServicesConcur*>* create(OptionalServicesConcurKey key) override;
  void destroy(OptionalServicesConcurKey key, std::vector<OptionalServicesConcur*>* recs) override;
  void load() override;

  static std::string _name;
  static std::string _cacheClass;
  static DAOHelper<OptionalServicesConcurDAO> _helper;
  static OptionalServicesConcurDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

class OptionalServicesConcurHistoricalDAO
    : public HistoricalDataAccessObject<OptionalServicesConcurHistoricalKey,
                                        std::vector<OptionalServicesConcur*> >
{
public:
  friend class DAOHelper<OptionalServicesConcurHistoricalDAO>;

  static OptionalServicesConcurHistoricalDAO& instance();

  const std::vector<OptionalServicesConcur*>& get(DeleteList& del,
                                                  const VendorCode& vendor,
                                                  const CarrierCode& carrier,
                                                  const ServiceTypeCode& serviceTypeCode,
                                                  const DateTime& date,
                                                  const DateTime& ticketDate);

  bool
  translateKey(const ObjectKey& objectKey, OptionalServicesConcurHistoricalKey& key) const override
  {
    objectKey.getValue("SERVICETYPECODE", key._c);
    objectKey.getValue("STARTDATE", key._d);
    objectKey.getValue("ENDDATE", key._e);

    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b);
  }

  bool translateKey(const ObjectKey& objectKey,
                    OptionalServicesConcurHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);

    objectKey.getValue("SERVICETYPECODE", key._c);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b);
  }

  virtual size_t invalidate(const ObjectKey& objectKey) override;

  const std::string& cacheClass() override { return _cacheClass; }

  void setKeyDateRange(OptionalServicesConcurHistoricalKey& key, const DateTime ticketDate) const
      override
  {
    DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

private:
  OptionalServicesConcurHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<OptionalServicesConcurHistoricalKey,
                                 std::vector<OptionalServicesConcur*> >(cacheSize, cacheType)
  {
  }

  std::vector<OptionalServicesConcur*>* create(OptionalServicesConcurHistoricalKey key) override;
  void destroy(OptionalServicesConcurHistoricalKey key,
               std::vector<OptionalServicesConcur*>* t) override;

  static std::string _name;
  static std::string _cacheClass;
  static DAOHelper<OptionalServicesConcurHistoricalDAO> _helper;
  static OptionalServicesConcurHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // tse

