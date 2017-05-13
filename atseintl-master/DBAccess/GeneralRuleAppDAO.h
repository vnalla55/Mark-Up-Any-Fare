//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/HashKey.h"
#include "DBAccess/HistoricalDataAccessObject.h"

#include <log4cxx/helpers/objectptr.h>

#include <vector>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class GeneralRuleApp;
class DeleteList;

typedef HashKey<VendorCode, CarrierCode, TariffNumber, RuleNumber> GeneralRuleAppKey;

class GeneralRuleAppDAO : public DataAccessObject<GeneralRuleAppKey, std::vector<GeneralRuleApp*> >
{
public:
  static GeneralRuleAppDAO& instance();
  const std::vector<GeneralRuleApp*>& get(DeleteList& del,
                                          const VendorCode& vendor,
                                          const CarrierCode& carrier,
                                          const TariffNumber& tariffNumber,
                                          const RuleNumber& ruleNumber,
                                          const DateTime& ticketDate);

  GeneralRuleApp* get(DeleteList& del,
                      const VendorCode& vendor,
                      const CarrierCode& carrier,
                      const TariffNumber& tariffNumber,
                      const RuleNumber& ruleNumber,
                      const DateTime& ticketDate,
                      CatNumber catNum);

  bool getTariffRule(DeleteList& del,
                     const VendorCode& vendor,
                     const CarrierCode& carrier,
                     const TariffNumber& tariffNumber,
                     const RuleNumber& ruleNumber,
                     const DateTime& ticketDate,
                     CatNumber catNum,
                     RuleNumber& ruleNumOut,
                     TariffNumber& tariffNumOut);

  const std::vector<GeneralRuleApp*>& getByTvlDate(DeleteList& del,
                                          const VendorCode& vendor,
                                          const CarrierCode& carrier,
                                          const TariffNumber& tariffNumber,
                                          const RuleNumber& ruleNumber,
                                          const DateTime& ticketDate,
                                          DateTime tvlDate);

  GeneralRuleApp* getByTvlDate(DeleteList& del,
                               const VendorCode& vendor,
                               const CarrierCode& carrier,
                               const TariffNumber& tariffNumber,
                               const RuleNumber& ruleNumber,
                               const DateTime& ticketDate,
                               CatNumber catNum,
                               DateTime tvlDate);

  bool getTariffRuleByTvlDate(DeleteList& del,
                              const VendorCode& vendor,
                              const CarrierCode& carrier,
                              const TariffNumber& tariffNumber,
                              const RuleNumber& ruleNumber,
                              const DateTime& ticketDate,
                              CatNumber catNum,
                              RuleNumber& ruleNumOut,
                              TariffNumber& tariffNumOut,
                              DateTime tvlDate);

  bool translateKey(const ObjectKey& objectKey, GeneralRuleAppKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b) &&
               objectKey.getValue("RULETARIFF", key._c) && objectKey.getValue("RULE", key._d);
  }

  GeneralRuleAppKey createKey(GeneralRuleApp* info);

  void translateKey(const GeneralRuleAppKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("CARRIER", key._b);
    objectKey.setValue("RULETARIFF", key._c);
    objectKey.setValue("RULE", key._d);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<GeneralRuleApp, GeneralRuleAppDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<GeneralRuleApp*>* vect) const override;

  virtual std::vector<GeneralRuleApp*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<GeneralRuleAppDAO>;
  static DAOHelper<GeneralRuleAppDAO> _helper;
  GeneralRuleAppDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<GeneralRuleAppKey, std::vector<GeneralRuleApp*> >(cacheSize, cacheType, 3)
  {
  }
  void load() override;
  std::vector<GeneralRuleApp*>* create(GeneralRuleAppKey key) override;
  void destroy(GeneralRuleAppKey key, std::vector<GeneralRuleApp*>* t) override;

private:
  static GeneralRuleAppDAO* _instance;
  struct isEffective;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: GeneralRuleAppHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, CarrierCode, TariffNumber, RuleNumber, DateTime, DateTime>
GeneralRuleAppHistoricalKey;

class GeneralRuleAppHistoricalDAO
    : public HistoricalDataAccessObject<GeneralRuleAppHistoricalKey, std::vector<GeneralRuleApp*> >
{
public:
  static GeneralRuleAppHistoricalDAO& instance();
  const std::vector<GeneralRuleApp*>& get(DeleteList& del,
                                          const VendorCode& vendor,
                                          const CarrierCode& carrier,
                                          const TariffNumber& tariffNumber,
                                          const RuleNumber& ruleNumber,
                                          const DateTime& ticketDate);

  GeneralRuleApp* get(DeleteList& del,
                      const VendorCode& vendor,
                      const CarrierCode& carrier,
                      const TariffNumber& tariffNumber,
                      const RuleNumber& ruleNumber,
                      const DateTime& ticketDate,
                      CatNumber catNum);

  bool getTariffRule(DeleteList& del,
                     const VendorCode& vendor,
                     const CarrierCode& carrier,
                     const TariffNumber& tariffNumber,
                     const RuleNumber& ruleNumber,
                     const DateTime& ticketDate,
                     CatNumber catNum,
                     RuleNumber& ruleNumOut,
                     TariffNumber& tariffNumOut);


  const std::vector<GeneralRuleApp*>& getByTvlDate(DeleteList& del,
                                                   const VendorCode& vendor,
                                                   const CarrierCode& carrier,
                                                   const TariffNumber& tariffNumber,
                                                   const RuleNumber& ruleNumber,
                                                   const DateTime& ticketDate,
                                                   DateTime tvlDate);

   GeneralRuleApp* getByTvlDate(DeleteList& del,
                                const VendorCode& vendor,
                                const CarrierCode& carrier,
                                const TariffNumber& tariffNumber,
                                const RuleNumber& ruleNumber,
                                const DateTime& ticketDate,
                                CatNumber catNum,
                                DateTime tvlDate);

   bool getTariffRuleByTvlDate(DeleteList& del,
                               const VendorCode& vendor,
                               const CarrierCode& carrier,
                               const TariffNumber& tariffNumber,
                               const RuleNumber& ruleNumber,
                               const DateTime& ticketDate,
                               CatNumber catNum,
                               RuleNumber& ruleNumOut,
                               TariffNumber& tariffNumOut,
                               DateTime tvlDate);

   bool translateKey(const ObjectKey& objectKey, GeneralRuleAppHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b) &&
               objectKey.getValue("RULETARIFF", key._c) && objectKey.getValue("RULE", key._d) &&
               objectKey.getValue("STARTDATE", key._e) && objectKey.getValue("ENDDATE", key._f);
  }

  bool translateKey(const ObjectKey& objectKey,
                    GeneralRuleAppHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._e, key._f, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b) &&
               objectKey.getValue("RULETARIFF", key._c) && objectKey.getValue("RULE", key._d);
  }

  void setKeyDateRange(GeneralRuleAppHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._e, key._f, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  GeneralRuleAppHistoricalKey createKey(const GeneralRuleApp*,
                                        const DateTime& startDate = DateTime::emptyDate(),
                                        const DateTime& endDate = DateTime::emptyDate());

  void translateKey(const GeneralRuleAppHistoricalKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("CARRIER", key._b);
    objectKey.setValue("RULETARIFF", key._c);
    objectKey.setValue("RULE", key._d);
    objectKey.setValue("STARTDATE", key._e);
    objectKey.setValue("ENDDATE", key._f);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserterWithDateRange<GeneralRuleApp, GeneralRuleAppHistoricalDAO>(
               flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  virtual sfc::CompressedData* compress(const std::vector<GeneralRuleApp*>* vect) const override;

  virtual std::vector<GeneralRuleApp*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<GeneralRuleAppHistoricalDAO>;
  static DAOHelper<GeneralRuleAppHistoricalDAO> _helper;
  GeneralRuleAppHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<GeneralRuleAppHistoricalKey, std::vector<GeneralRuleApp*> >(
          cacheSize, cacheType, 3)
  {
  }
  std::vector<GeneralRuleApp*>* create(GeneralRuleAppHistoricalKey key) override;
  void destroy(GeneralRuleAppHistoricalKey key, std::vector<GeneralRuleApp*>* t) override;
  void load() override;

private:
  static GeneralRuleAppHistoricalDAO* _instance;
  struct isEffective;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
