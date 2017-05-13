// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------
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

class AirlineAllianceCarrierInfo;
class DeleteList;

typedef HashKey<CarrierCode> CarrierKey;

class AirlineAllianceCarrierDAO
    : public DataAccessObject<CarrierKey, std::vector<AirlineAllianceCarrierInfo*> >
{
public:
  static AirlineAllianceCarrierDAO& instance();

  const std::vector<AirlineAllianceCarrierInfo*>&
  get(DeleteList& del, const CarrierCode& carrierCode, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, CarrierKey& key) const override
  {
    return (key.initialized = (objectKey.getValue("CARRIERCODE", key._a)));
  }

  CarrierKey createKey(AirlineAllianceCarrierInfo* info);

  void translateKey(const CarrierKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CARRIERCODE", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<AirlineAllianceCarrierInfo, AirlineAllianceCarrierDAO>(
               flatKey, objectKey).success();
  }

  virtual size_t invalidate(const ObjectKey& objectKey) override;
  const std::string& cacheClass() override { return _cacheClass; }
  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<AirlineAllianceCarrierDAO>;
  static DAOHelper<AirlineAllianceCarrierDAO> _helper;
  AirlineAllianceCarrierDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<CarrierKey, std::vector<AirlineAllianceCarrierInfo*> >(cacheSize, cacheType, 2)
  {
  }
  std::vector<AirlineAllianceCarrierInfo*>* create(CarrierKey key) override;
  void destroy(CarrierKey key, std::vector<AirlineAllianceCarrierInfo*>* recs) override;

  void load() override;

private:
  static AirlineAllianceCarrierDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

typedef HashKey<CarrierCode, DateTime, DateTime> CarrierHistoricalKey;

class AirlineAllianceCarrierHistoricalDAO
    : public HistoricalDataAccessObject<CarrierHistoricalKey,
                                        std::vector<AirlineAllianceCarrierInfo*> >
{
public:
  static AirlineAllianceCarrierHistoricalDAO& instance();

  const std::vector<AirlineAllianceCarrierInfo*>&
  get(DeleteList& del, const CarrierCode& carrierCode, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, CarrierHistoricalKey& key) const override
  {
    key.initialized = objectKey.getValue("CARRIERCODE", key._a);
    if (key.initialized && (_cacheBy != DAOUtils::NODATES))
    {
      key.initialized =
          objectKey.getValue("STARTDATE", key._b) && objectKey.getValue("ENDDATE", key._c);
    }
    return key.initialized;
  }

  bool translateKey(const ObjectKey& objectKey,
                    CarrierHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
    return key.initialized = objectKey.getValue("CARRIERCODE", key._a);
  }

  void setKeyDateRange(CarrierHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  }

  virtual size_t invalidate(const ObjectKey& objectKey) override;
  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<AirlineAllianceCarrierHistoricalDAO>;
  static DAOHelper<AirlineAllianceCarrierHistoricalDAO> _helper;
  AirlineAllianceCarrierHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<CarrierHistoricalKey, std::vector<AirlineAllianceCarrierInfo*> >(
          cacheSize, cacheType, 2)
  {
  }
  std::vector<AirlineAllianceCarrierInfo*>* create(CarrierHistoricalKey key) override;
  void destroy(CarrierHistoricalKey key, std::vector<AirlineAllianceCarrierInfo*>* t) override;

private:
  static AirlineAllianceCarrierHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
}

