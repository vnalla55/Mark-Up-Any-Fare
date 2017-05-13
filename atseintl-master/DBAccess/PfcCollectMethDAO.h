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
class PfcCollectMeth;
class DeleteList;

class PfcCollectMethDAO : public DataAccessObject<CarrierKey, std::vector<PfcCollectMeth*>, false>
{
public:
  static PfcCollectMethDAO& instance();
  const std::vector<PfcCollectMeth*>&
  get(DeleteList& del, const CarrierCode& key, const DateTime& date, const DateTime& ticketDate);

  const std::vector<PfcCollectMeth*>& getAll(DeleteList& del);

  bool translateKey(const ObjectKey& objectKey, CarrierKey& key) const override
  {
    return key.initialized = objectKey.getValue("CARRIER", key._a);
  }

  CarrierKey createKey(PfcCollectMeth* info);

  void translateKey(const CarrierKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CARRIER", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<PfcCollectMeth, PfcCollectMethDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<PfcCollectMethDAO>;
  static DAOHelper<PfcCollectMethDAO> _helper;
  PfcCollectMethDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<CarrierKey, std::vector<PfcCollectMeth*>, false>(cacheSize, cacheType)
  {
  }
  virtual void load() override;
  std::vector<PfcCollectMeth*>* create(CarrierKey key) override;
  void destroy(CarrierKey key, std::vector<PfcCollectMeth*>* t) override;

private:
  struct isEffective;
  static PfcCollectMethDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
// --------------------------------------------------
// Historical DAO: PfcCollectMethHistoricalDAO
// --------------------------------------------------
typedef HashKey<CarrierCode, DateTime, DateTime> PfcCollectMethHistoricalKey;

class PfcCollectMethHistoricalDAO : public HistoricalDataAccessObject<PfcCollectMethHistoricalKey,
                                                                      std::vector<PfcCollectMeth*>,
                                                                      false>
{
public:
  static PfcCollectMethHistoricalDAO& instance();
  const std::vector<PfcCollectMeth*>&
  get(DeleteList& del, const CarrierCode& key, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, PfcCollectMethHistoricalKey& key) const override
  {
    return key.initialized = objectKey.getValue("CARRIER", key._a) &&
                             objectKey.getValue("STARTDATE", key._b) &&
                             objectKey.getValue("ENDDATE", key._c);
  }

  bool translateKey(const ObjectKey& objectKey,
                    PfcCollectMethHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
    return key.initialized = objectKey.getValue("CARRIER", key._a);
  }

  void setKeyDateRange(PfcCollectMethHistoricalKey& key, const DateTime date) const override
  {
    DAOUtils::getDateRange(date, key._b, key._c, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<PfcCollectMethHistoricalDAO>;
  static DAOHelper<PfcCollectMethHistoricalDAO> _helper;
  PfcCollectMethHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<PfcCollectMethHistoricalKey, std::vector<PfcCollectMeth*>, false>(
          cacheSize, cacheType)
  {
  }
  std::vector<PfcCollectMeth*>* create(PfcCollectMethHistoricalKey key) override;
  void destroy(PfcCollectMethHistoricalKey key, std::vector<PfcCollectMeth*>* t) override;

private:
  struct isEffective;
  static PfcCollectMethHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
