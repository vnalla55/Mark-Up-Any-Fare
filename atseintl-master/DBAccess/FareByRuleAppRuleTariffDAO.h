//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/HistoricalDataAccessObject.h"

#include <log4cxx/helpers/objectptr.h>

namespace tse
{
class FareByRuleApp;
class CorpId;
class DeleteList;

typedef HashKey<CarrierCode, TariffNumber> FareByRuleAppRuleTariffKey;

class FareByRuleAppRuleTariffDAO
    : public DataAccessObject<FareByRuleAppRuleTariffKey, std::vector<FareByRuleApp*> >
{
public:
  static FareByRuleAppRuleTariffDAO& instance();

  // retrieve all FareByRuleApp matching CorpIds with no AccountCode
  const std::vector<FareByRuleApp*>& get(DeleteList& del,
                                         const CarrierCode& carrier,
                                         const TktDesignator& tktDesignator,
                                         const std::vector<PaxTypeCode>& paxTypes,
                                         const std::vector<CorpId*>& corpIds,
                                         const DateTime& tvlDate,
                                         const DateTime& ticketDate,
                                         bool isFareDisplay);

  bool translateKey(const ObjectKey& objectKey, FareByRuleAppRuleTariffKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("CARRIER", key._a) && objectKey.getValue("RULETARIFF", key._b);
  }

  FareByRuleAppRuleTariffKey createKey(const FareByRuleApp* info);

  void translateKey(const FareByRuleAppRuleTariffKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CARRIER", key._a);
    objectKey.setValue("RULETARIFF", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<FareByRuleApp, FareByRuleAppRuleTariffDAO>(flatKey, objectKey)
        .success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual size_t invalidate(const ObjectKey& objectKey) override;

  virtual sfc::CompressedData* compress(const std::vector<FareByRuleApp*>* vect) const override;

  virtual std::vector<FareByRuleApp*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareByRuleAppRuleTariffDAO>;
  static DAOHelper<FareByRuleAppRuleTariffDAO> _helper;
  FareByRuleAppRuleTariffDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FareByRuleAppRuleTariffKey, std::vector<FareByRuleApp*> >(
          cacheSize, cacheType, 3)
  {
  }
  std::vector<FareByRuleApp*>* create(FareByRuleAppRuleTariffKey key) override;
  void destroy(FareByRuleAppRuleTariffKey key, std::vector<FareByRuleApp*>* recs) override;
  void load() override;

private:
  struct FilterPaxType;
  static FareByRuleAppRuleTariffDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

typedef HashKey<CarrierCode, TariffNumber, DateTime, DateTime> FareByRuleAppRuleTariffHistoricalKey;

class FareByRuleAppRuleTariffHistoricalDAO
    : public HistoricalDataAccessObject<FareByRuleAppRuleTariffHistoricalKey,
                                        std::vector<FareByRuleApp*> >
{
public:
  static FareByRuleAppRuleTariffHistoricalDAO& instance();

  // retrieve all FareByRuleApp matching CorpIds with no AccountCode
  const std::vector<FareByRuleApp*>& get(DeleteList& del,
                                         const CarrierCode& carrier,
                                         const TktDesignator& tktDesignator,
                                         const std::vector<PaxTypeCode>& paxTypes,
                                         const std::vector<CorpId*>& corpIds,
                                         const DateTime& tvlDate,
                                         const DateTime& ticketDate,
                                         bool isFareDisplay);

  bool
  translateKey(const ObjectKey& objectKey, FareByRuleAppRuleTariffHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._c);
    objectKey.getValue("ENDDATE", key._d);
    return key.initialized = objectKey.getValue("CARRIER", key._a)
                             && objectKey.getValue("RULETARIFF", key._b);
  }

  bool translateKey(const ObjectKey& objectKey,
                    FareByRuleAppRuleTariffHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("CARRIER", key._a) && objectKey.getValue("RULETARIFF", key._b);
  }

  void setKeyDateRange(FareByRuleAppRuleTariffHistoricalKey& key, const DateTime ticketDate) const
      override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual size_t invalidate(const ObjectKey& objectKey) override;

  virtual sfc::CompressedData* compress(const std::vector<FareByRuleApp*>* vect) const override;

  virtual std::vector<FareByRuleApp*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareByRuleAppRuleTariffHistoricalDAO>;
  static DAOHelper<FareByRuleAppRuleTariffHistoricalDAO> _helper;
  FareByRuleAppRuleTariffHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FareByRuleAppRuleTariffHistoricalKey,
                                 std::vector<FareByRuleApp*> >(cacheSize, cacheType, 3)
  {
  }
  std::vector<FareByRuleApp*>* create(FareByRuleAppRuleTariffHistoricalKey key) override;
  void
  destroy(FareByRuleAppRuleTariffHistoricalKey key, std::vector<FareByRuleApp*>* recs) override;

private:
  struct FilterPaxType;
  static FareByRuleAppRuleTariffHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
