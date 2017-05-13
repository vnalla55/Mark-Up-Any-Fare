//-------------------------------------------------------------------------------
// Copyright 2015, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
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
class InterlineCarrierInfo;
class DeleteList;

class InterlineCarrierDAO :
      public DataAccessObject<CarrierKey, std::vector<InterlineCarrierInfo*>, false>
{
public:
  static InterlineCarrierDAO& instance();

  const std::vector<InterlineCarrierInfo*>&
    get(DeleteList& del);

  bool translateKey(const ObjectKey& objectKey, CarrierKey& key) const override
  {
    return key.initialized = objectKey.getValue("CARRIER", WHOLE_TBL_KEY._a);
  }

  CarrierKey createKey(InterlineCarrierInfo* info) { return WHOLE_TBL_KEY; }

  void translateKey(const CarrierKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CARRIER", WHOLE_TBL_KEY._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<InterlineCarrierInfo, InterlineCarrierDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual size_t clear() override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<InterlineCarrierDAO>;
  static DAOHelper<InterlineCarrierDAO> _helper;
  InterlineCarrierDAO(int cacheSize = 0, const std::string& cacheType = "", size_t version = 2)
    : DataAccessObject<CarrierKey, std::vector<InterlineCarrierInfo*>, false>(
          cacheSize, cacheType, version)
  {
  }
  virtual void load() override;
  std::vector<InterlineCarrierInfo*>* create(CarrierKey key) override;
  void destroy(CarrierKey key, std::vector<InterlineCarrierInfo*>* recs) override;

private:
  static CarrierKey WHOLE_TBL_KEY;
  static InterlineCarrierDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class InterlineCarrierDAO

// --------------------------------------------------
// Historical DAO: InterlineCarrierHistoricalDAO
// --------------------------------------------------
class InterlineCarrierHistoricalDAO :
      public HistoricalDataAccessObject<CarrierCode, std::vector<InterlineCarrierInfo*>, false>
{
public:
  static InterlineCarrierHistoricalDAO& instance();
  const std::vector<InterlineCarrierInfo*>&
   get(DeleteList& del);

  bool translateKey(const ObjectKey& objectKey, CarrierCode& key) const override
  {
    return objectKey.getValue("CARRIER", key);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<InterlineCarrierHistoricalDAO>;
  static DAOHelper<InterlineCarrierHistoricalDAO> _helper;
  InterlineCarrierHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<CarrierCode, std::vector<InterlineCarrierInfo*>, false>(cacheSize,
                                                                                         cacheType)
  {
  }
  virtual void load() override;
  std::vector<InterlineCarrierInfo*>* create(CarrierCode key) override;
  void destroy(CarrierCode key, std::vector<InterlineCarrierInfo*>* recs) override;

private:
  struct groupByKey;
  static InterlineCarrierHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class InterlineCarrierHistoricalDAO
} // namespace tse
