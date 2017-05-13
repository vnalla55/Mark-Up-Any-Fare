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
class CarrierMixedClass;
class DeleteList;

class CarrierMixedClassDAO
    : public DataAccessObject<CarrierKey, std::vector<CarrierMixedClass*>, false>
{
public:
  static CarrierMixedClassDAO& instance();

  const std::vector<CarrierMixedClass*>&
  get(DeleteList& del, const CarrierCode& key, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, CarrierKey& key) const override
  {
    return key.initialized = objectKey.getValue("CARRIER", key._a);
  }

  CarrierKey createKey(CarrierMixedClass* info);

  void translateKey(const CarrierKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CARRIER", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<CarrierMixedClass, CarrierMixedClassDAO>(flatKey, objectKey)
        .success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<CarrierMixedClassDAO>;

  static DAOHelper<CarrierMixedClassDAO> _helper;

  CarrierMixedClassDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<CarrierKey, std::vector<CarrierMixedClass*>, false>(cacheSize, cacheType)
  {
  }

  std::vector<CarrierMixedClass*>* create(CarrierKey key) override;
  void destroy(CarrierKey key, std::vector<CarrierMixedClass*>* recs) override;

  virtual void load() override;

private:
  struct isEffective;
  static CarrierMixedClassDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
// --------------------------------------------------
// Historical DAO: CarrierMixedClassHistoricalDAO
// --------------------------------------------------
class CarrierMixedClassHistoricalDAO
    : public HistoricalDataAccessObject<CarrierCode, std::vector<CarrierMixedClass*>, false>
{
public:
  static CarrierMixedClassHistoricalDAO& instance();

  const std::vector<CarrierMixedClass*>&
  get(DeleteList& del, const CarrierCode& key, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, CarrierCode& key) const override
  {
    return objectKey.getValue("CARRIER", key);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<CarrierMixedClassHistoricalDAO>;

  static DAOHelper<CarrierMixedClassHistoricalDAO> _helper;

  CarrierMixedClassHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<CarrierCode, std::vector<CarrierMixedClass*>, false>(cacheSize,
                                                                                      cacheType)
  {
  }

  std::vector<CarrierMixedClass*>* create(CarrierCode key) override;
  void destroy(CarrierCode key, std::vector<CarrierMixedClass*>* recs) override;

  virtual void load() override;

private:
  struct isEffective;
  struct groupByKey;
  static CarrierMixedClassHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
