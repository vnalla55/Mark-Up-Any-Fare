//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
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
class Routing;
class DeleteList;

typedef HashKey<VendorCode, CarrierCode, TariffNumber, RoutingNumber> RoutingKey;

class RoutingDAO : public DataAccessObject<RoutingKey, std::vector<Routing*> >
{
public:
  static RoutingDAO& instance();

  const std::vector<Routing*>& get(DeleteList& del,
                                   const VendorCode& vendor,
                                   const CarrierCode& carrier,
                                   const TariffNumber& routingTariff,
                                   const RoutingNumber& routingNumber,
                                   const DateTime& date,
                                   const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, RoutingKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b) &&
               objectKey.getValue("ROUTINGTARIFF", key._c) && objectKey.getValue("ROUTING", key._d);
  }

  RoutingKey createKey(Routing* info);

  void translateKey(const RoutingKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("CARRIER", key._b);
    objectKey.setValue("ROUTINGTARIFF", key._c);
    objectKey.setValue("ROUTING", key._d);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<Routing, RoutingDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<Routing*>* vect) const override;

  virtual std::vector<Routing*>* uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<RoutingDAO>;
  static DAOHelper<RoutingDAO> _helper;
  RoutingDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<RoutingKey, std::vector<Routing*> >(cacheSize, cacheType, 3)
  {
  }
  void load() override;
  std::vector<Routing*>* create(RoutingKey key) override;
  void destroy(RoutingKey key, std::vector<Routing*>* t) override;

private:
  static RoutingDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class RoutingDAO

// Historical Stuff /////////////////////////////////////////////////////////////
typedef HashKey<VendorCode, CarrierCode, TariffNumber, RoutingNumber, DateTime, DateTime>
RoutingHistoricalKey;

class RoutingHistoricalDAO
    : public HistoricalDataAccessObject<RoutingHistoricalKey, std::vector<Routing*> >
{
public:
  static RoutingHistoricalDAO& instance();
  static log4cxx::LoggerPtr _logger;

  const std::vector<Routing*>& get(DeleteList& del,
                                   const VendorCode& vendor,
                                   const CarrierCode& carrier,
                                   const TariffNumber& routingTariff,
                                   const RoutingNumber& routingNumber,
                                   const DateTime& date,
                                   const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, RoutingHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._e);
    objectKey.getValue("ENDDATE", key._f);
    return key.initialized = objectKey.getValue("VENDOR", key._a)
                             && objectKey.getValue("CARRIER", key._b)
                             && objectKey.getValue("ROUTINGTARIFF", key._c)
                             && objectKey.getValue("ROUTING", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    RoutingHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._e, key._f, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b) &&
               objectKey.getValue("ROUTINGTARIFF", key._c) && objectKey.getValue("ROUTING", key._d);
  }

  RoutingHistoricalKey createKey(const Routing* info,
                                 const DateTime& startDate = DateTime::emptyDate(),
                                 const DateTime& endDate = DateTime::emptyDate());

  void translateKey(const RoutingHistoricalKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("CARRIER", key._b);
    objectKey.setValue("ROUTINGTARIFF", key._c);
    objectKey.setValue("ROUTING", key._d);
    objectKey.setValue("STARTDATE", key._e);
    objectKey.setValue("ENDDATE", key._f);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserterWithDateRange<Routing, RoutingHistoricalDAO>(flatKey, objectKey)
        .success();
  }

  void setKeyDateRange(RoutingHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._e, key._f, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  virtual sfc::CompressedData* compress(const std::vector<Routing*>* vect) const override;

  virtual std::vector<Routing*>* uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<RoutingHistoricalDAO>;
  static DAOHelper<RoutingHistoricalDAO> _helper;
  RoutingHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<RoutingHistoricalKey, std::vector<Routing*> >(cacheSize, cacheType, 3)
  {
  }
  std::vector<Routing*>* create(RoutingHistoricalKey key) override;
  void destroy(RoutingHistoricalKey key, std::vector<Routing*>* t) override;
  void load() override;

private:
  static RoutingHistoricalDAO* _instance;
}; // class RoutingHistoricalDAO
} // namespace tse
