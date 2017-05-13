//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
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
class Differentials;
class DeleteList;

class DifferentialsDAO : public DataAccessObject<CarrierKey, std::vector<Differentials*>, false>
{
public:
  static DifferentialsDAO& instance();
  const std::vector<Differentials*>&
  get(DeleteList& del, const CarrierCode& key, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, CarrierKey& key) const override
  {
    return key.initialized = objectKey.getValue("CARRIER", key._a);
  }

  CarrierKey createKey(Differentials* info);

  void translateKey(const CarrierKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CARRIER", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<Differentials, DifferentialsDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<DifferentialsDAO>;
  static DAOHelper<DifferentialsDAO> _helper;
  DifferentialsDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<CarrierKey, std::vector<Differentials*>, false>(cacheSize, cacheType, 2)
  {
  }
  std::vector<Differentials*>* create(CarrierKey key) override;
  void destroy(CarrierKey key, std::vector<Differentials*>* recs) override;

  virtual void load() override;

private:
  struct isCalc;
  static DifferentialsDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: DifferentialsHistoricalDAO
// --------------------------------------------------
class DifferentialsHistoricalDAO
    : public HistoricalDataAccessObject<CarrierCode, std::vector<Differentials*>, false>
{
public:
  static DifferentialsHistoricalDAO& instance();
  const std::vector<Differentials*>&
  get(DeleteList& del, const CarrierCode& key, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, CarrierCode& key) const override
  {
    return objectKey.getValue("CARRIER", key);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<DifferentialsHistoricalDAO>;
  static DAOHelper<DifferentialsHistoricalDAO> _helper;
  DifferentialsHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<CarrierCode, std::vector<Differentials*>, false>(
          cacheSize, cacheType, 2)
  {
  }
  std::vector<Differentials*>* create(CarrierCode key) override;
  void destroy(CarrierCode key, std::vector<Differentials*>* recs) override;

  virtual void load() override;

private:
  struct isCalc;
  struct groupByKey;
  static DifferentialsHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse

