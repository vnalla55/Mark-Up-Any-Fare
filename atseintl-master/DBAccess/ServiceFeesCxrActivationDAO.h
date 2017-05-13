//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc. Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOUtils.h"
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
class ServiceFeesCxrActivation;
class DeleteList;

typedef HashKey<CarrierCode> ServiceFeesCxrActivationKey;

class ServiceFeesCxrActivationDAO
    : public DataAccessObject<ServiceFeesCxrActivationKey, std::vector<ServiceFeesCxrActivation*> >
{
public:
  static ServiceFeesCxrActivationDAO& instance();

  const std::vector<ServiceFeesCxrActivation*>& get(DeleteList& del,
                                                    const CarrierCode& validatingCarrier,
                                                    const DateTime& date,
                                                    const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, ServiceFeesCxrActivationKey& key) const override
  {
    return (key.initialized = (objectKey.getValue("CARRIER", key._a)));
  }

  ServiceFeesCxrActivationKey createKey(ServiceFeesCxrActivation* info);

  void translateKey(const ServiceFeesCxrActivationKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CARRIER", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<ServiceFeesCxrActivation, ServiceFeesCxrActivationDAO>(
               flatKey, objectKey).success();
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<ServiceFeesCxrActivationDAO>;
  static DAOHelper<ServiceFeesCxrActivationDAO> _helper;
  ServiceFeesCxrActivationDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<ServiceFeesCxrActivationKey, std::vector<ServiceFeesCxrActivation*> >(
          cacheSize, cacheType)
  {
  }
  std::vector<ServiceFeesCxrActivation*>* create(ServiceFeesCxrActivationKey key) override;
  void
  destroy(ServiceFeesCxrActivationKey key, std::vector<ServiceFeesCxrActivation*>* recs) override;

  void load() override;

private:
  static ServiceFeesCxrActivationDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

typedef HashKey<CarrierCode, DateTime, DateTime> ServiceFeesCxrActivationHistoricalKey;

class ServiceFeesCxrActivationHistoricalDAO
    : public HistoricalDataAccessObject<ServiceFeesCxrActivationHistoricalKey,
                                        std::vector<ServiceFeesCxrActivation*> >
{
public:
  static ServiceFeesCxrActivationHistoricalDAO& instance();

  const std::vector<ServiceFeesCxrActivation*>& get(DeleteList& del,
                                                    const CarrierCode& carrier,
                                                    const DateTime& date,
                                                    const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, ServiceFeesCxrActivationHistoricalKey& key) const
      override
  {
    return key.initialized = objectKey.getValue("CARRIER", key._a) &&
                             objectKey.getValue("STARTDATE", key._b) &&
                             objectKey.getValue("ENDDATE", key._c);
  }

  bool translateKey(const ObjectKey& objectKey,
                    ServiceFeesCxrActivationHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
    return key.initialized = objectKey.getValue("CARRIER", key._a);
  }

  void setKeyDateRange(ServiceFeesCxrActivationHistoricalKey& key, const DateTime ticketDate) const
      override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<ServiceFeesCxrActivationHistoricalDAO>;
  static DAOHelper<ServiceFeesCxrActivationHistoricalDAO> _helper;
  ServiceFeesCxrActivationHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<ServiceFeesCxrActivationHistoricalKey,
                                 std::vector<ServiceFeesCxrActivation*> >(cacheSize, cacheType)
  {
  }
  std::vector<ServiceFeesCxrActivation*>*
  create(ServiceFeesCxrActivationHistoricalKey key) override;
  void destroy(ServiceFeesCxrActivationHistoricalKey key,
               std::vector<ServiceFeesCxrActivation*>* t) override;

private:
  static ServiceFeesCxrActivationHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
}
