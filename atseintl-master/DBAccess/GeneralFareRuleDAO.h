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
#include "DBAccess/DeleteList.h"
#include "DBAccess/HistoricalDataAccessObject.h"

namespace tse
{
class GeneralFareRuleInfo;
class DeleteList;

typedef HashKey<VendorCode, CarrierCode, TariffNumber, RuleNumber, CatNumber> GeneralFareRuleKey;

class GeneralFareRuleDAO
    : public DataAccessObject<GeneralFareRuleKey, std::vector<GeneralFareRuleInfo*> >
{
  friend class DAOHelper<GeneralFareRuleDAO>;

public:
  static GeneralFareRuleDAO& instance();

  const std::vector<GeneralFareRuleInfo*>& getAll(const VendorCode vendor,
                                                  const CarrierCode carrier,
                                                  const TariffNumber ruleTariff,
                                                  const RuleNumber rule,
                                                  const CatNumber category)
  {
    // Track calls for code coverage
    _codeCoverageGetCallCount++;

    return *getFromCache(GeneralFareRuleKey(vendor, carrier, ruleTariff, rule, category)).get();
  }

  const std::vector<GeneralFareRuleInfo*>& get(DeleteList& del,
                                               const VendorCode& vendor,
                                               const CarrierCode& carrier,
                                               const TariffNumber& ruleTariff,
                                               const RuleNumber& rule,
                                               const CatNumber& category,
                                               const DateTime& date,
                                               const DateTime& ticketDate);

  const std::vector<GeneralFareRuleInfo*>& getForFD(DeleteList& del,
                                                    const VendorCode& vendor,
                                                    const CarrierCode& carrier,
                                                    const TariffNumber& ruleTariff,
                                                    const RuleNumber& rule,
                                                    const CatNumber& category,
                                                    const DateTime& date,
                                                    const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, GeneralFareRuleKey& key) const override
  {
    return key.initialized =
               (objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b) &&
                objectKey.getValue("RULETARIFF", key._c) && objectKey.getValue("RULE", key._d) &&
                objectKey.getValue("CATEGORY", key._e));
  }

  GeneralFareRuleKey createKey(GeneralFareRuleInfo* info);

  void translateKey(const GeneralFareRuleKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("CARRIER", key._b);
    objectKey.setValue("RULETARIFF", key._c);
    objectKey.setValue("RULE", key._d);
    objectKey.setValue("CATEGORY", key._e);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<GeneralFareRuleInfo, GeneralFareRuleDAO>(flatKey, objectKey)
        .success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData*
  compress(const std::vector<GeneralFareRuleInfo*>* vect) const override;

  virtual std::vector<GeneralFareRuleInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  GeneralFareRuleDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<GeneralFareRuleKey, std::vector<GeneralFareRuleInfo*> >(
          cacheSize, cacheType, 6),
      _loadedOnStartup(false)
  {
  }

  std::vector<GeneralFareRuleInfo*>* create(GeneralFareRuleKey key) override;
  void destroy(GeneralFareRuleKey key, std::vector<GeneralFareRuleInfo*>* t) override;

  void load() override;
  virtual size_t clear() override;

  DAOCache::pointer_type getFromCache(const GeneralFareRuleKey& key)
  {
    if (UNLIKELY(_loadOnUpdate && _loadedOnStartup))
    {
      return cache().getIfResident(key);
    }
    else
    {
      return cache().get(key);
    }
  }

  static std::string _name;
  static std::string _cacheClass;
  static DAOHelper<GeneralFareRuleDAO> _helper;
  bool _loadedOnStartup;

private:
  struct isEffective;
  static GeneralFareRuleDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

typedef HashKey<VendorCode, CarrierCode, TariffNumber, RuleNumber, CatNumber, DateTime, DateTime>
GeneralFareRuleHistoricalKey;

class GeneralFareRuleHistoricalDAO
    : public HistoricalDataAccessObject<GeneralFareRuleHistoricalKey,
                                        std::vector<GeneralFareRuleInfo*> >
{
  friend class DAOHelper<GeneralFareRuleHistoricalDAO>;

public:
  static GeneralFareRuleHistoricalDAO& instance();

  const std::vector<GeneralFareRuleInfo*>& get(DeleteList& del,
                                               const VendorCode& vendor,
                                               const CarrierCode& carrier,
                                               const TariffNumber& ruleTariff,
                                               const RuleNumber& rule,
                                               const CatNumber& category,
                                               const DateTime& date,
                                               const DateTime& ticketDate);

  const std::vector<GeneralFareRuleInfo*>& getForFD(DeleteList& del,
                                                    const VendorCode& vendor,
                                                    const CarrierCode& carrier,
                                                    const TariffNumber& ruleTariff,
                                                    const RuleNumber& rule,
                                                    const CatNumber& category,
                                                    const DateTime& date,
                                                    const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, GeneralFareRuleHistoricalKey& key) const override
  {
    return key.initialized =
               (objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b) &&
                objectKey.getValue("RULETARIFF", key._c) && objectKey.getValue("RULE", key._d) &&
                objectKey.getValue("CATEGORY", key._e) && objectKey.getValue("STARTDATE", key._f) &&
                objectKey.getValue("ENDDATE", key._g));
  }

  bool translateKey(const ObjectKey& objectKey,
                    GeneralFareRuleHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._f, key._g, _cacheBy);
    return key.initialized =
               (objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b) &&
                objectKey.getValue("RULETARIFF", key._c) && objectKey.getValue("RULE", key._d) &&
                objectKey.getValue("CATEGORY", key._e));
  }

  void translateKey(const GeneralFareRuleHistoricalKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("CARRIER", key._b);
    objectKey.setValue("RULETARIFF", key._c);
    objectKey.setValue("RULE", key._d);
    objectKey.setValue("CATEGORY", key._e);
    objectKey.setValue("STARTDATE", key._f);
    objectKey.setValue("ENDDATE", key._g);
  }

  void setKeyDateRange(GeneralFareRuleHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._f, key._g, _cacheBy);
  }

  GeneralFareRuleHistoricalKey createKey(const GeneralFareRuleInfo* info,
                                         const DateTime& startDate = DateTime::emptyDate(),
                                         const DateTime& endDate = DateTime::emptyDate());

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserterWithDateRange<GeneralFareRuleInfo, GeneralFareRuleHistoricalDAO>(
               flatKey, objectKey).success();
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  virtual sfc::CompressedData*
  compress(const std::vector<GeneralFareRuleInfo*>* vect) const override;

  virtual std::vector<GeneralFareRuleInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  GeneralFareRuleHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<GeneralFareRuleHistoricalKey, std::vector<GeneralFareRuleInfo*> >(
          cacheSize, cacheType, 6),
      _loadedOnStartup(false)
  {
  }

  std::vector<GeneralFareRuleInfo*>* create(GeneralFareRuleHistoricalKey key) override;
  void destroy(GeneralFareRuleHistoricalKey key, std::vector<GeneralFareRuleInfo*>* t) override;

  void load() override;
  virtual size_t clear() override;

  DAOCache::pointer_type getFromCache(const GeneralFareRuleHistoricalKey& key)
  {
    if (_loadOnUpdate && _loadedOnStartup)
    {
      return cache().getIfResident(key);
    }
    else
    {
      return cache().get(key);
    }
  }

  static std::string _name;
  static std::string _cacheClass;
  static DAOHelper<GeneralFareRuleHistoricalDAO> _helper;
  bool _loadedOnStartup;

private:
  static GeneralFareRuleHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

} // namespace tse
