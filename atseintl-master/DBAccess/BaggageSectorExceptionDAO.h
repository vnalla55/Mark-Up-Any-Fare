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
class BaggageSectorException;
class DeleteList;

typedef HashKey<CarrierCode> BSEKey;

class BaggageSectorExceptionDAO
    : public DataAccessObject<BSEKey, std::vector<BaggageSectorException*>, false>
{
public:
  static BaggageSectorExceptionDAO& instance();

  const std::vector<BaggageSectorException*>&
  get(DeleteList& del, const CarrierCode& carrier, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, BSEKey& key) const override
  {
    return key.initialized = objectKey.getValue("CARRIER", key._a);
  }

  BSEKey createKey(BaggageSectorException* info);

  void translateKey(const BSEKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CARRIER", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<BaggageSectorException, BaggageSectorExceptionDAO>(
               flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<BaggageSectorExceptionDAO>;

  static DAOHelper<BaggageSectorExceptionDAO> _helper;

  BaggageSectorExceptionDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<BSEKey, std::vector<BaggageSectorException*>, false>(cacheSize, cacheType)
  {
  }

  std::vector<BaggageSectorException*>* create(BSEKey key) override;
  void destroy(BSEKey key, std::vector<BaggageSectorException*>* recs) override;

  virtual void load() override;

private:
  static BaggageSectorExceptionDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: BaggageSectorExceptionHistoricalDAO
// --------------------------------------------------
class BaggageSectorExceptionHistoricalDAO
    : public HistoricalDataAccessObject<BSEKey, std::vector<BaggageSectorException*>, false>
{
public:
  static BaggageSectorExceptionHistoricalDAO& instance();

  const std::vector<BaggageSectorException*>&
  get(DeleteList& del, const CarrierCode& carrier, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, BSEKey& key) const override
  {
    return key.initialized = objectKey.getValue("CARRIER", key._a);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<BaggageSectorExceptionHistoricalDAO>;

  static DAOHelper<BaggageSectorExceptionHistoricalDAO> _helper;

  BaggageSectorExceptionHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<BSEKey, std::vector<BaggageSectorException*>, false>(cacheSize,
                                                                                      cacheType)
  {
  }

  std::vector<BaggageSectorException*>* create(BSEKey key) override;
  void destroy(BSEKey key, std::vector<BaggageSectorException*>* recs) override;

  virtual void load() override;

private:
  struct groupByKey;
  static BaggageSectorExceptionHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse

