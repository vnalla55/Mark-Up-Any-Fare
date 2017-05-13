//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#pragma once

#include "Common/TseCodeTypes.h"
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
class CarrierPreference;
class DeleteList;

class CarrierPreferenceDAO
    : public DataAccessObject<CarrierKey, std::vector<CarrierPreference*>, false>
{
public:
  static CarrierPreferenceDAO& instance();

  const CarrierPreference*
  get(DeleteList& del, const CarrierCode& key, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, CarrierKey& key) const override
  {
    return key.initialized = objectKey.getValue("CARRIER", key._a);
  }

  CarrierKey createKey(CarrierPreference* info);

  void translateKey(const CarrierKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CARRIER", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<CarrierPreference, CarrierPreferenceDAO>(flatKey, objectKey)
        .success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<CarrierPreferenceDAO>;

  static DAOHelper<CarrierPreferenceDAO> _helper;

  CarrierPreferenceDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<CarrierKey, std::vector<CarrierPreference*>, false>(cacheSize, cacheType, 7)
  {
  }

  std::vector<CarrierPreference*>* create(CarrierKey key) override;
  void destroy(CarrierKey key, std::vector<CarrierPreference*>* recs) override;

  virtual void load() override;

private:
  static CarrierPreferenceDAO* _instance;
  struct isEffective;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: CarrierPreferenceHistoricalDAO
// --------------------------------------------------
class CarrierPreferenceHistoricalDAO
    : public HistoricalDataAccessObject<CarrierCode, std::vector<CarrierPreference*>, false>
{
public:
  static CarrierPreferenceHistoricalDAO& instance();

  const CarrierPreference*
  get(DeleteList& del, const CarrierCode& key, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, CarrierCode& key) const override
  {
    return objectKey.getValue("CARRIER", key);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<CarrierPreferenceHistoricalDAO>;

  static DAOHelper<CarrierPreferenceHistoricalDAO> _helper;

  CarrierPreferenceHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<CarrierCode, std::vector<CarrierPreference*>, false>(
          cacheSize, cacheType, 7)
  {
  }

  std::vector<CarrierPreference*>* create(CarrierCode key) override;
  void destroy(CarrierCode key, std::vector<CarrierPreference*>* recs) override;

  virtual void load() override;

private:
  static CarrierPreferenceHistoricalDAO* _instance;
  struct isEffective;
  struct groupByKey;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse

