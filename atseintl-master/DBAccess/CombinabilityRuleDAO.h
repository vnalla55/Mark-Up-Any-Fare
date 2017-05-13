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

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class CombinabilityRuleInfo;
class DeleteList;

typedef HashKey<VendorCode, CarrierCode, TariffNumber, RuleNumber> CombinabilityRuleKey;

class CombinabilityRuleDAO
    : public DataAccessObject<CombinabilityRuleKey, std::vector<CombinabilityRuleInfo*> >
{
public:
  static CombinabilityRuleDAO& instance();

  const std::vector<CombinabilityRuleInfo*>& get(DeleteList& del,
                                                 const VendorCode& vendor,
                                                 const CarrierCode& carrier,
                                                 const TariffNumber& ruleTariff,
                                                 const RuleNumber& rule,
                                                 const DateTime& date,
                                                 const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, CombinabilityRuleKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b) &&
               objectKey.getValue("RULETARIFF", key._c) && objectKey.getValue("RULE", key._d);
  }

  CombinabilityRuleKey createKey(CombinabilityRuleInfo* info);

  void translateKey(const CombinabilityRuleKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("CARRIER", key._b);
    objectKey.setValue("RULETARIFF", key._c);
    objectKey.setValue("RULE", key._d);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<CombinabilityRuleInfo, CombinabilityRuleDAO>(flatKey, objectKey)
        .success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData*
  compress(const std::vector<CombinabilityRuleInfo*>* vect) const override;

  virtual std::vector<CombinabilityRuleInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<CombinabilityRuleDAO>;
  static DAOHelper<CombinabilityRuleDAO> _helper;
  CombinabilityRuleDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<CombinabilityRuleKey, std::vector<CombinabilityRuleInfo*>>(
          cacheSize, cacheType, 6)
  {
  }
  void load() override;
  std::vector<CombinabilityRuleInfo*>* create(CombinabilityRuleKey key) override;
  void destroy(CombinabilityRuleKey key, std::vector<CombinabilityRuleInfo*>* t) override;

private:
  static CombinabilityRuleDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// Historical Stuff
// //////////////////////////////////////////////////////////////////////////////////////////
typedef HashKey<VendorCode, CarrierCode, TariffNumber, RuleNumber, DateTime, DateTime>
CombinabilityRuleHistoricalKey;

class CombinabilityRuleHistoricalDAO
    : public HistoricalDataAccessObject<CombinabilityRuleHistoricalKey,
                                        std::vector<CombinabilityRuleInfo*> >
{
public:
  static CombinabilityRuleHistoricalDAO& instance();

  const std::vector<CombinabilityRuleInfo*>& get(DeleteList& del,
                                                 const VendorCode& vendor,
                                                 const CarrierCode& carrier,
                                                 const TariffNumber& ruleTariff,
                                                 const RuleNumber& rule,
                                                 const DateTime& date,
                                                 const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, CombinabilityRuleHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._e);
    objectKey.getValue("ENDDATE", key._f);
    return key.initialized = objectKey.getValue("VENDOR", key._a)
                             && objectKey.getValue("CARRIER", key._b)
                             && objectKey.getValue("RULETARIFF", key._c)
                             && objectKey.getValue("RULE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    CombinabilityRuleHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._e, key._f, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b) &&
               objectKey.getValue("RULETARIFF", key._c) && objectKey.getValue("RULE", key._d);
  }

  CombinabilityRuleHistoricalKey createKey(const CombinabilityRuleInfo* info,
                                           const DateTime& startDate = DateTime::emptyDate(),
                                           const DateTime& endDate = DateTime::emptyDate());

  void translateKey(const CombinabilityRuleHistoricalKey& key, ObjectKey& objectKey) const override
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
    return DummyObjectInserterWithDateRange<CombinabilityRuleInfo, CombinabilityRuleHistoricalDAO>(
               flatKey, objectKey).success();
  }

  void
  setKeyDateRange(CombinabilityRuleHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._e, key._f, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  virtual sfc::CompressedData*
  compress(const std::vector<CombinabilityRuleInfo*>* vect) const override;

  virtual std::vector<CombinabilityRuleInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<CombinabilityRuleHistoricalDAO>;
  static DAOHelper<CombinabilityRuleHistoricalDAO> _helper;
  CombinabilityRuleHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<CombinabilityRuleHistoricalKey,
                                 std::vector<CombinabilityRuleInfo*> >(cacheSize, cacheType, 5)
  {
  }
  std::vector<CombinabilityRuleInfo*>* create(CombinabilityRuleHistoricalKey key) override;
  void destroy(CombinabilityRuleHistoricalKey key, std::vector<CombinabilityRuleInfo*>* t) override;
  void load() override;

private:
  static CombinabilityRuleHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

} // namespace tse
