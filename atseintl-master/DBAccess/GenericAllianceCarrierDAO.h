// ----------------------------------------------------------------
//
//   Copyright Sabre 2014
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

namespace tse
{
class AirlineAllianceCarrierInfo;
class DeleteList;
class Logger;

typedef HashKey<GenericAllianceCode> GenericCarrierKey;

class GenericAllianceCarrierDAO
    : public DataAccessObject<GenericCarrierKey, std::vector<AirlineAllianceCarrierInfo*> >
{
public:
  static GenericAllianceCarrierDAO& instance();

  const std::vector<AirlineAllianceCarrierInfo*>&
  get(DeleteList& del, const CarrierCode& carrierCode, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, GenericCarrierKey& key) const override
  {
    return (key.initialized = (objectKey.getValue("GENERICALLIANCECODE", key._a)));
  }

  GenericCarrierKey createKey(AirlineAllianceCarrierInfo* info);

  void translateKey(const GenericCarrierKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("GENERICALLIANCECODE", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<AirlineAllianceCarrierInfo, GenericAllianceCarrierDAO>(
               flatKey, objectKey).success();
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<GenericAllianceCarrierDAO>;
  static DAOHelper<GenericAllianceCarrierDAO> _helper;
  GenericAllianceCarrierDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<GenericCarrierKey, std::vector<AirlineAllianceCarrierInfo*> >(cacheSize,
                                                                                     cacheType)
  {
  }
  std::vector<AirlineAllianceCarrierInfo*>* create(GenericCarrierKey key) override;
  void destroy(GenericCarrierKey key, std::vector<AirlineAllianceCarrierInfo*>* recs) override;

  void load() override;

private:
  static GenericAllianceCarrierDAO* _instance;
  static Logger _logger;
};

typedef HashKey<GenericAllianceCode, DateTime, DateTime> GenericCarrierHistoricalKey;

class GenericAllianceCarrierHistoricalDAO
    : public HistoricalDataAccessObject<GenericCarrierHistoricalKey,
                                        std::vector<AirlineAllianceCarrierInfo*> >
{
public:
  static GenericAllianceCarrierHistoricalDAO& instance();

  const std::vector<AirlineAllianceCarrierInfo*>&
  get(DeleteList& del, const CarrierCode& carrierCode, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, GenericCarrierHistoricalKey& key) const override
  {
    key.initialized = objectKey.getValue("GENERICALLIANCECODE", key._a);
    if (key.initialized && (_cacheBy != DAOUtils::NODATES))
    {
      key.initialized =
          objectKey.getValue("STARTDATE", key._b) && objectKey.getValue("ENDDATE", key._c);
    }
    return key.initialized;
  }

  bool translateKey(const ObjectKey& objectKey,
                    GenericCarrierHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
    return key.initialized = objectKey.getValue("GENERICALLIANCECODE", key._a);
  }

  void setKeyDateRange(GenericCarrierHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<GenericAllianceCarrierHistoricalDAO>;
  static DAOHelper<GenericAllianceCarrierHistoricalDAO> _helper;
  GenericAllianceCarrierHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<GenericCarrierHistoricalKey,
                                 std::vector<AirlineAllianceCarrierInfo*> >(cacheSize, cacheType)
  {
  }
  std::vector<AirlineAllianceCarrierInfo*>* create(GenericCarrierHistoricalKey key) override;
  void
  destroy(GenericCarrierHistoricalKey key, std::vector<AirlineAllianceCarrierInfo*>* t) override;

private:
  static GenericAllianceCarrierHistoricalDAO* _instance;
  static Logger _logger;
};
}

