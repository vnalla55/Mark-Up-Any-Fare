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
class FareByRuleCtrlInfo;
class DeleteList;

typedef HashKey<VendorCode, CarrierCode, TariffNumber, RuleNumber> FareByRuleCtrlKey;

class FareByRuleCtrlDAO
    : public DataAccessObject<FareByRuleCtrlKey, std::vector<FareByRuleCtrlInfo*> >
{
public:
  static FareByRuleCtrlDAO& instance();

  const std::vector<FareByRuleCtrlInfo*>& getAll(const VendorCode& vendor,
                                                 const CarrierCode& carrier,
                                                 const TariffNumber& tariffNumber,
                                                 const RuleNumber& ruleNumber)
  {
    // Track calls for code coverage
    _codeCoverageGetCallCount++;

    return *cache().get(FareByRuleCtrlKey(vendor, carrier, tariffNumber, ruleNumber));
  }

  const std::vector<FareByRuleCtrlInfo*>& get(DeleteList& del,
                                              const VendorCode& vendor,
                                              const CarrierCode& carrier,
                                              const TariffNumber& tariffNumber,
                                              const RuleNumber& ruleNumber,
                                              const DateTime& tvlDate,
                                              const DateTime& ticketDate);

  const std::vector<FareByRuleCtrlInfo*>& getForFD(DeleteList& del,
                                                   const VendorCode& vendor,
                                                   const CarrierCode& carrier,
                                                   const TariffNumber& tariffNumber,
                                                   const RuleNumber& ruleNumber,
                                                   const DateTime& date,
                                                   const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareByRuleCtrlKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b) &&
               objectKey.getValue("RULETARIFF", key._c) && objectKey.getValue("RULE", key._d);
  }

  FareByRuleCtrlKey createKey(FareByRuleCtrlInfo* info);

  void translateKey(const FareByRuleCtrlKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("CARRIER", key._b);
    objectKey.setValue("RULETARIFF", key._c);
    objectKey.setValue("RULE", key._d);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<FareByRuleCtrlInfo, FareByRuleCtrlDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData*
  compress(const std::vector<FareByRuleCtrlInfo*>* vect) const override;

  virtual std::vector<FareByRuleCtrlInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareByRuleCtrlDAO>;
  static DAOHelper<FareByRuleCtrlDAO> _helper;
  FareByRuleCtrlDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FareByRuleCtrlKey, std::vector<FareByRuleCtrlInfo*> >(
          cacheSize, cacheType, 3)
  {
  }
  void load() override;
  std::vector<FareByRuleCtrlInfo*>* create(FareByRuleCtrlKey key) override;
  void destroy(FareByRuleCtrlKey key, std::vector<FareByRuleCtrlInfo*>* t) override;

private:
  static FareByRuleCtrlDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

typedef HashKey<VendorCode, CarrierCode, TariffNumber, RuleNumber, DateTime, DateTime>
FareByRuleCtrlHistoricalKey;

class FareByRuleCtrlHistoricalDAO
    : public HistoricalDataAccessObject<FareByRuleCtrlHistoricalKey,
                                        std::vector<FareByRuleCtrlInfo*> >
{
public:
  static FareByRuleCtrlHistoricalDAO& instance();

  const std::vector<FareByRuleCtrlInfo*>& get(DeleteList& del,
                                              const VendorCode& vendor,
                                              const CarrierCode& carrier,
                                              const TariffNumber& tariffNumber,
                                              const RuleNumber& rule,
                                              const DateTime& tvlDate,
                                              const DateTime& ticketDate);

  const std::vector<FareByRuleCtrlInfo*>& getForFD(DeleteList& del,
                                                   const VendorCode& vendor,
                                                   const CarrierCode& carrier,
                                                   const TariffNumber& tariffNumber,
                                                   const RuleNumber& ruleNumber,
                                                   const DateTime& date,
                                                   const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareByRuleCtrlHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._e);
    objectKey.getValue("ENDDATE", key._f);
    return key.initialized = objectKey.getValue("VENDOR", key._a)
                             && objectKey.getValue("CARRIER", key._b)
                             && objectKey.getValue("RULETARIFF", key._c)
                             && objectKey.getValue("RULE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    FareByRuleCtrlHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._e, key._f, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b) &&
               objectKey.getValue("RULETARIFF", key._c) && objectKey.getValue("RULE", key._d);
  }

  FareByRuleCtrlHistoricalKey createKey(FareByRuleCtrlInfo* info,
                                        const DateTime& startDate = DateTime::emptyDate(),
                                        const DateTime& endDate = DateTime::emptyDate());

  void translateKey(const FareByRuleCtrlHistoricalKey& key, ObjectKey& objectKey) const override
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
    return DummyObjectInserterWithDateRange<FareByRuleCtrlInfo, FareByRuleCtrlHistoricalDAO>(
               flatKey, objectKey).success();
  }

  void setKeyDateRange(FareByRuleCtrlHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._e, key._f, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  virtual sfc::CompressedData*
  compress(const std::vector<FareByRuleCtrlInfo*>* vect) const override;

  virtual std::vector<FareByRuleCtrlInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareByRuleCtrlHistoricalDAO>;
  static DAOHelper<FareByRuleCtrlHistoricalDAO> _helper;
  FareByRuleCtrlHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FareByRuleCtrlHistoricalKey, std::vector<FareByRuleCtrlInfo*> >(
          cacheSize, cacheType, 3)
  {
  }
  std::vector<FareByRuleCtrlInfo*>* create(FareByRuleCtrlHistoricalKey key) override;
  void destroy(FareByRuleCtrlHistoricalKey key, std::vector<FareByRuleCtrlInfo*>* t) override;
  void load() override;

private:
  static FareByRuleCtrlHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
