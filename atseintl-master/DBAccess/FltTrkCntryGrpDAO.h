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
class FltTrkCntryGrp;
class DeleteList;

class FltTrkCntryGrpDAO : public DataAccessObject<CarrierKey, std::vector<FltTrkCntryGrp*>, false>
{
public:
  static FltTrkCntryGrpDAO& instance();

  const FltTrkCntryGrp*
  get(DeleteList& del, const CarrierCode& key, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, CarrierKey& key) const override
  {
    return key.initialized = objectKey.getValue("CARRIER", key._a);
  }

  CarrierKey createKey(FltTrkCntryGrp* info);

  void translateKey(const CarrierKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CARRIER", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<FltTrkCntryGrp, FltTrkCntryGrpDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<FltTrkCntryGrpDAO>;

  static DAOHelper<FltTrkCntryGrpDAO> _helper;

  FltTrkCntryGrpDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<CarrierKey, std::vector<FltTrkCntryGrp*>, false>(cacheSize, cacheType)
  {
  }

  std::vector<FltTrkCntryGrp*>* create(CarrierKey key) override;
  void destroy(CarrierKey key, std::vector<FltTrkCntryGrp*>* recs) override;

  virtual void load() override;

private:
  static FltTrkCntryGrpDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: FltTrkCntryGrpHistoricalDAO
// --------------------------------------------------
class FltTrkCntryGrpHistoricalDAO
    : public HistoricalDataAccessObject<CarrierCode, std::vector<FltTrkCntryGrp*>, false>
{
public:
  static FltTrkCntryGrpHistoricalDAO& instance();

  const FltTrkCntryGrp*
  get(DeleteList& del, const CarrierCode& key, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, CarrierCode& key) const override
  {
    return objectKey.getValue("CARRIER", key);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<FltTrkCntryGrpHistoricalDAO>;

  static DAOHelper<FltTrkCntryGrpHistoricalDAO> _helper;

  FltTrkCntryGrpHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<CarrierCode, std::vector<FltTrkCntryGrp*>, false>(cacheSize,
                                                                                   cacheType)
  {
  }

  std::vector<FltTrkCntryGrp*>* create(CarrierCode key) override;
  void destroy(CarrierCode key, std::vector<FltTrkCntryGrp*>* recs) override;

  virtual void load() override;

private:
  static FltTrkCntryGrpHistoricalDAO* _instance;
  struct groupByKey;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
