//-------------------------------------------------------------------------------
// Copyright 2013, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
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
class GenSalesAgentInfo;
class DeleteList;

typedef HashKey<CrsCode, NationCode, SettlementPlanType, CarrierCode> GenSalesAgentKey;

class GenSalesAgentDAO : public DataAccessObject<GenSalesAgentKey, std::vector<GenSalesAgentInfo*> >
{
public:
  static GenSalesAgentDAO& instance();
  const std::vector<GenSalesAgentInfo*>& get(DeleteList& del,
                                             const CrsCode& gds,
                                             const NationCode& country,
                                             const SettlementPlanType& settlementPlan,
                                             const CarrierCode& validatingCxr,
                                             const DateTime& ticketDate);

  const std::vector<GenSalesAgentInfo*>& get(DeleteList& del,
                                             const CrsCode& gds,
                                             const NationCode& country,
                                             const SettlementPlanType& settlementPlan,
                                             const DateTime& date);

  GenSalesAgentKey createKey(GenSalesAgentInfo* info);
  bool translateKey(const ObjectKey& objectKey, GenSalesAgentKey& key) const override;
  void translateKey(const GenSalesAgentKey& key, ObjectKey& objectKey) const override;

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<GenSalesAgentInfo, GenSalesAgentDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }
  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<GenSalesAgentDAO>;
  static DAOHelper<GenSalesAgentDAO> _helper;

  GenSalesAgentDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<GenSalesAgentKey, std::vector<GenSalesAgentInfo*> >(cacheSize, cacheType, 1)
  {
  }

  std::vector<GenSalesAgentInfo*>* create(GenSalesAgentKey key) override;
  void destroy(GenSalesAgentKey key, std::vector<GenSalesAgentInfo*>* t) override;
  void load() override;
  virtual size_t clear() override;

private:
  static GenSalesAgentDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

//-----------------------------------------------------------------------------
// HISTORICAL
//-----------------------------------------------------------------------------

typedef HashKey<CrsCode, NationCode, SettlementPlanType, CarrierCode, DateTime, DateTime>
    GenSalesAgentHistoricalKey;

class GenSalesAgentHistoricalDAO
    : public HistoricalDataAccessObject<GenSalesAgentHistoricalKey,
                                        std::vector<GenSalesAgentInfo*> >
{
public:
  static GenSalesAgentHistoricalDAO& instance();
  const std::vector<GenSalesAgentInfo*>& get(DeleteList& del,
                                             const CrsCode& gds,
                                             const NationCode& country,
                                             const SettlementPlanType& settlementPlan,
                                             const CarrierCode& validatingCxr,
                                             const DateTime& ticketDate);

  GenSalesAgentHistoricalKey createKey(GenSalesAgentInfo* info);
  bool translateKey(const ObjectKey& objectKey, GenSalesAgentHistoricalKey& key) const override;
  void translateKey(const GenSalesAgentHistoricalKey& key, ObjectKey& objectKey) const override;
  bool translateKey(const ObjectKey& objectKey,
                    GenSalesAgentHistoricalKey& key,
                    const DateTime ticketDate) const override;

  log4cxx::LoggerPtr& getLogger() override { return _logger; }
  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<GenSalesAgentHistoricalDAO>;
  static DAOHelper<GenSalesAgentHistoricalDAO> _helper;

  GenSalesAgentHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<GenSalesAgentHistoricalKey,
                                 std::vector<GenSalesAgentInfo*> >(cacheSize, cacheType, 1)
  {
  }

  std::vector<GenSalesAgentInfo*>* create(GenSalesAgentHistoricalKey key) override;
  void destroy(GenSalesAgentHistoricalKey key, std::vector<GenSalesAgentInfo*>* gsaList) override;

private:
  static GenSalesAgentHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

} // namespace tse
