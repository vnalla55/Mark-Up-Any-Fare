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
class CommissionCap;
class DeleteList;

typedef HashKey<CarrierCode, CurrencyCode> CommissionCapKey;

class CommissionCapDAO
    : public DataAccessObject<CommissionCapKey, std::vector<CommissionCap*>, false>
{
public:
  static CommissionCapDAO& instance();

  const std::vector<CommissionCap*>& get(DeleteList& del,
                                         const CarrierCode& carrier,
                                         const CurrencyCode& cur,
                                         const DateTime& date,
                                         const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, CommissionCapKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("CARRIER", key._a) && objectKey.getValue("CURRENCYCODE", key._b);
  }

  CommissionCapKey createKey(CommissionCap* info);

  void translateKey(const CommissionCapKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CARRIER", key._a);
    objectKey.setValue("CURRENCYCODE", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<CommissionCap, CommissionCapDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<CommissionCapDAO>;

  static DAOHelper<CommissionCapDAO> _helper;

  CommissionCapDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<CommissionCapKey, std::vector<CommissionCap*>, false>(cacheSize, cacheType)
  {
  }

  std::vector<CommissionCap*>* create(CommissionCapKey key) override;
  void destroy(CommissionCapKey key, std::vector<CommissionCap*>* recs) override;

  virtual void load() override;

private:
  struct isEffective;
  struct groupByKey;
  static CommissionCapDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
// --------------------------------------------------
// Historical DAO: CommissionCapHistoricalDAO
// --------------------------------------------------
class CommissionCapHistoricalDAO
    : public HistoricalDataAccessObject<CommissionCapKey, std::vector<CommissionCap*>, false>
{
public:
  static CommissionCapHistoricalDAO& instance();

  const std::vector<CommissionCap*>& get(DeleteList& del,
                                         const CarrierCode& carrier,
                                         const CurrencyCode& cur,
                                         const DateTime& date,
                                         const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, CommissionCapKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("CARRIER", key._a) && objectKey.getValue("CURRENCYCODE", key._b);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<CommissionCapHistoricalDAO>;

  static DAOHelper<CommissionCapHistoricalDAO> _helper;

  CommissionCapHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<CommissionCapKey, std::vector<CommissionCap*>, false>(cacheSize,
                                                                                       cacheType)
  {
  }

  std::vector<CommissionCap*>* create(CommissionCapKey key) override;
  void destroy(CommissionCapKey key, std::vector<CommissionCap*>* recs) override;

  virtual void load() override;

private:
  struct isEffective;
  struct groupByKey;
  static CommissionCapHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
