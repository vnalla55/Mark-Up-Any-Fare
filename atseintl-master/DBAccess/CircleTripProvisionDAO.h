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
class CircleTripProvision;
class DeleteList;

typedef HashKey<LocCode, LocCode> CircleTripProvisionKey;

class CircleTripProvisionDAO
    : public DataAccessObject<CircleTripProvisionKey, std::vector<CircleTripProvision*>, false>
{
public:
  static CircleTripProvisionDAO& instance();

  const CircleTripProvision* get(DeleteList& del,
                                 const LocCode& market1,
                                 const LocCode& market2,
                                 const DateTime& date,
                                 const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, CircleTripProvisionKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("MARKET1", key._a) && objectKey.getValue("MARKET2", key._b);
  }

  CircleTripProvisionKey createKey(CircleTripProvision* info);

  void translateKey(const CircleTripProvisionKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("MARKET1", key._a);
    objectKey.setValue("MARKET2", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<CircleTripProvision, CircleTripProvisionDAO>(flatKey, objectKey)
        .success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData*
  compress(const std::vector<CircleTripProvision*>* vect) const override;

  virtual std::vector<CircleTripProvision*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<CircleTripProvisionDAO>;

  static DAOHelper<CircleTripProvisionDAO> _helper;

  CircleTripProvisionDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<CircleTripProvisionKey, std::vector<CircleTripProvision*>, false>(cacheSize,
                                                                                         cacheType)
  {
  }

  virtual void load() override;
  std::vector<CircleTripProvision*>* create(CircleTripProvisionKey key) override;
  void destroy(CircleTripProvisionKey key, std::vector<CircleTripProvision*>* t) override;

private:
  struct isEffective;
  static CircleTripProvisionDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: CircleTripProvisionHistoricalDAO
// --------------------------------------------------
class CircleTripProvisionHistoricalDAO
    : public HistoricalDataAccessObject<CircleTripProvisionKey,
                                        std::vector<CircleTripProvision*>,
                                        false>
{
public:
  static CircleTripProvisionHistoricalDAO& instance();

  const CircleTripProvision* get(DeleteList& del,
                                 const LocCode& market1,
                                 const LocCode& market2,
                                 const DateTime& date,
                                 const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, CircleTripProvisionKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("MARKET1", key._a) && objectKey.getValue("MARKET2", key._b);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData*
  compress(const std::vector<CircleTripProvision*>* vect) const override;

  virtual std::vector<CircleTripProvision*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<CircleTripProvisionHistoricalDAO>;

  static DAOHelper<CircleTripProvisionHistoricalDAO> _helper;

  CircleTripProvisionHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<CircleTripProvisionKey, std::vector<CircleTripProvision*>, false>(
          cacheSize, cacheType)
  {
  }

  std::vector<CircleTripProvision*>* create(CircleTripProvisionKey key) override;
  void destroy(CircleTripProvisionKey key, std::vector<CircleTripProvision*>* t) override;

  virtual void load() override;

private:
  struct isEffective;
  struct groupByKey;
  static CircleTripProvisionHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse

