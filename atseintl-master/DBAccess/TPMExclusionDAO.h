//------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
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
class TPMExclusion;
class DeleteList;

typedef HashKey<CarrierCode> TPMExclKey;

class TPMExclusionDAO : public DataAccessObject<TPMExclKey, std::vector<TPMExclusion*>, false>
{
public:
  static TPMExclusionDAO& instance();

  const std::vector<TPMExclusion*>&
  get(DeleteList& del, const CarrierCode& carrier, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, TPMExclKey& key) const override
  {
    return key.initialized = objectKey.getValue("CARRIER", key._a);
  }

  TPMExclKey createKey(TPMExclusion* info);

  void translateKey(const TPMExclKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CARRIER", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<TPMExclusion, TPMExclusionDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<TPMExclusionDAO>;

  static DAOHelper<TPMExclusionDAO> _helper;

  TPMExclusionDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<TPMExclKey, std::vector<TPMExclusion*>, false>(cacheSize, cacheType)
  {
  }

  std::vector<TPMExclusion*>* create(TPMExclKey key) override;
  void destroy(TPMExclKey key, std::vector<TPMExclusion*>* recs) override;

  virtual void load() override;

private:
  static TPMExclusionDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: TPMExclusionHistoricalDAO
// --------------------------------------------------
class TPMExclusionHistoricalDAO
    : public HistoricalDataAccessObject<TPMExclKey, std::vector<TPMExclusion*>, false>
{
public:
  static TPMExclusionHistoricalDAO& instance();

  const std::vector<TPMExclusion*>&
  get(DeleteList& del, const CarrierCode& carrier, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, TPMExclKey& key) const override
  {
    return key.initialized = objectKey.getValue("CARRIER", key._a);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<TPMExclusionHistoricalDAO>;

  static DAOHelper<TPMExclusionHistoricalDAO> _helper;

  TPMExclusionHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<TPMExclKey, std::vector<TPMExclusion*>, false>(cacheSize,
                                                                                cacheType)
  {
  }

  std::vector<TPMExclusion*>* create(TPMExclKey key) override;
  void destroy(TPMExclKey key, std::vector<TPMExclusion*>* recs) override;

  virtual void load() override;

private:
  struct groupByKey;
  static TPMExclusionHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse

