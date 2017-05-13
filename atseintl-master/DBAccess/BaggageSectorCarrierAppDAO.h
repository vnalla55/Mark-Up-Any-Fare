//------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//------------------------------------------------------------------------------
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
class BaggageSectorCarrierApp;
class DeleteList;

typedef HashKey<CarrierCode> Key;

class BaggageSectorCarrierAppDAO
    : public DataAccessObject<Key, std::vector<BaggageSectorCarrierApp*>, false>
{
public:
  static BaggageSectorCarrierAppDAO& instance();

  const std::vector<BaggageSectorCarrierApp*>&
  get(DeleteList& del, const CarrierCode& carrier, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, Key& key) const override
  {
    return key.initialized = objectKey.getValue("CARRIER", key._a);
  }

  Key createKey(BaggageSectorCarrierApp* info);

  void translateKey(const Key& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CARRIER", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<BaggageSectorCarrierApp, BaggageSectorCarrierAppDAO>(
               flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<BaggageSectorCarrierAppDAO>;

  static DAOHelper<BaggageSectorCarrierAppDAO> _helper;

  BaggageSectorCarrierAppDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<Key, std::vector<BaggageSectorCarrierApp*>, false>(cacheSize, cacheType)
  {
  }

  std::vector<BaggageSectorCarrierApp*>* create(Key key) override;
  void destroy(Key key, std::vector<BaggageSectorCarrierApp*>* recs) override;

  virtual void load() override;

private:
  static BaggageSectorCarrierAppDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: BaggageSectorCarrierAppHistoricalDAO
// --------------------------------------------------
class BaggageSectorCarrierAppHistoricalDAO
    : public HistoricalDataAccessObject<Key, std::vector<BaggageSectorCarrierApp*>, false>
{
public:
  static BaggageSectorCarrierAppHistoricalDAO& instance();

  const std::vector<BaggageSectorCarrierApp*>&
  get(DeleteList& del, const CarrierCode& carrier, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, Key& key) const override
  {
    return key.initialized = objectKey.getValue("CARRIER", key._a);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<BaggageSectorCarrierAppHistoricalDAO>;

  static DAOHelper<BaggageSectorCarrierAppHistoricalDAO> _helper;

  BaggageSectorCarrierAppHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<Key, std::vector<BaggageSectorCarrierApp*>, false>(cacheSize,
                                                                                    cacheType)
  {
  }

  std::vector<BaggageSectorCarrierApp*>* create(Key key) override;
  void destroy(Key key, std::vector<BaggageSectorCarrierApp*>* recs) override;

  virtual void load() override;

private:
  struct groupByKey;
  static BaggageSectorCarrierAppHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse

