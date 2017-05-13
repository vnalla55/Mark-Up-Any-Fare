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

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class FareClassAppInfo;
class DeleteList;

typedef HashKey<VendorCode, CarrierCode, TariffNumber, RuleNumber, FareClassCode> FareClassAppKey;

class FareClassAppDAO
    : public DataAccessObject<FareClassAppKey, std::vector<const FareClassAppInfo*> >
{
public:
  static FareClassAppDAO& instance();

  const std::vector<const FareClassAppInfo*>& get(DeleteList& del,
                                                  const VendorCode& vendor,
                                                  const CarrierCode& carrier,
                                                  const TariffNumber& ruleTariff,
                                                  const RuleNumber& ruleNumber,
                                                  const FareClassCode& fareClass,
                                                  const DateTime& ticketDate);

  const std::vector<const FareClassAppInfo*>& getByTravelDT(DeleteList& del,
                                                  const VendorCode& vendor,
                                                  const CarrierCode& carrier,
                                                  const TariffNumber& ruleTariff,
                                                  const RuleNumber& ruleNumber,
                                                  const FareClassCode& fareClass,
                                                  const DateTime& ticketDate,
                                                  const DateTime& travelDate);

  const std::vector<const FareClassAppInfo*>& getForFD(DeleteList& del,
                                                       const VendorCode& vendor,
                                                       const CarrierCode& carrier,
                                                       const TariffNumber& ruleTariff,
                                                       const RuleNumber& ruleNumber,
                                                       const FareClassCode& fareClass,
                                                       const DateTime& ticketDate);

  const std::vector<const FareClassAppInfo*>& getForFDByTravelDT(DeleteList& del,
                                                       const VendorCode& vendor,
                                                       const CarrierCode& carrier,
                                                       const TariffNumber& ruleTariff,
                                                       const RuleNumber& ruleNumber,
                                                       const FareClassCode& fareClass,
                                                       const DateTime& ticketDate,
                                                       const DateTime& travelDate);

  virtual void put(const std::vector<const FareClassAppInfo*>& recs);

  bool translateKey(const ObjectKey& objectKey, FareClassAppKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b) &&
               objectKey.getValue("RULETARIFF", key._c) && objectKey.getValue("RULE", key._d) &&
               objectKey.getValue("FARECLASS", key._e);
  }

  FareClassAppKey createKey(const FareClassAppInfo* info);

  void translateKey(const FareClassAppKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("CARRIER", key._b);
    objectKey.setValue("RULETARIFF", key._c);
    objectKey.setValue("RULE", key._d);
    objectKey.setValue("FARECLASS", key._e);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<FareClassAppInfo, FareClassAppDAO>(flatKey, objectKey).success();
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  virtual sfc::CompressedData*
  compress(const std::vector<const FareClassAppInfo*>* vect) const override;

  virtual std::vector<const FareClassAppInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareClassAppDAO>;
  static DAOHelper<FareClassAppDAO> _helper;
  FareClassAppDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FareClassAppKey, std::vector<const FareClassAppInfo*> >(
          cacheSize, cacheType, 4)
  {
  }
  std::vector<const FareClassAppInfo*>* create(FareClassAppKey key) override;
  void destroy(FareClassAppKey key, std::vector<const FareClassAppInfo*>* recs) override;

  void load() override;
  virtual size_t clear() override;

private:
  struct groupByKey;
  struct isNotApplicable;
  struct isNotApplicableByTravelDT;
  struct isNotApplicableForFD;
  struct isNotApplicableForFDByTravelDT;
  static FareClassAppDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// Historical Stuff /////////////////////////////////////////////////////////////
typedef HashKey<VendorCode,
                CarrierCode,
                TariffNumber,
                RuleNumber,
                FareClassCode,
                DateTime,
                DateTime> FareClassAppHistoricalKey;

class FareClassAppHistoricalDAO
    : public HistoricalDataAccessObject<FareClassAppHistoricalKey,
                                        std::vector<const FareClassAppInfo*> >
{
public:
  static FareClassAppHistoricalDAO& instance();

  const std::vector<const FareClassAppInfo*>& get(DeleteList& del,
                                                  const VendorCode& vendor,
                                                  const CarrierCode& carrier,
                                                  const TariffNumber& ruleTariff,
                                                  const RuleNumber& ruleNumber,
                                                  const FareClassCode& fareClass,
                                                  const DateTime& ticketDate);

  const std::vector<const FareClassAppInfo*>& getByTravelDT(DeleteList& del,
                                                  const VendorCode& vendor,
                                                  const CarrierCode& carrier,
                                                  const TariffNumber& ruleTariff,
                                                  const RuleNumber& ruleNumber,
                                                  const FareClassCode& fareClass,
                                                  const DateTime& ticketDate,
                                                  const DateTime& travelDate);

  const std::vector<const FareClassAppInfo*>& getForFD(DeleteList& del,
                                                       const VendorCode& vendor,
                                                       const CarrierCode& carrier,
                                                       const TariffNumber& ruleTariff,
                                                       const RuleNumber& ruleNumber,
                                                       const FareClassCode& fareClass,
                                                       const DateTime& ticketDate);

  const std::vector<const FareClassAppInfo*>& getForFDByTravelDT(DeleteList& del,
                                                       const VendorCode& vendor,
                                                       const CarrierCode& carrier,
                                                       const TariffNumber& ruleTariff,
                                                       const RuleNumber& ruleNumber,
                                                       const FareClassCode& fareClass,
                                                       const DateTime& ticketDate,
                                                       const DateTime& travelDate);

  bool translateKey(const ObjectKey& objectKey, FareClassAppHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._f);
    objectKey.getValue("ENDDATE", key._g);
    return key.initialized = objectKey.getValue("VENDOR", key._a)
                             && objectKey.getValue("CARRIER", key._b)
                             && objectKey.getValue("RULETARIFF", key._c)
                             && objectKey.getValue("RULE", key._d)
                             && objectKey.getValue("FARECLASS", key._e);
  }

  bool translateKey(const ObjectKey& objectKey,
                    FareClassAppHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._f, key._g, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b) &&
               objectKey.getValue("RULETARIFF", key._c) && objectKey.getValue("RULE", key._d) &&
               objectKey.getValue("FARECLASS", key._e);
  }

  void translateKey(const FareClassAppHistoricalKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("CARRIER", key._b);
    objectKey.setValue("RULETARIFF", key._c);
    objectKey.setValue("RULE", key._d);
    objectKey.setValue("FARECLASS", key._e);
    objectKey.setValue("STARTDATE", key._f);
    objectKey.setValue("ENDDATE", key._g);
  }

  void setKeyDateRange(FareClassAppHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._f, key._g, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  FareClassAppHistoricalKey createKey(const FareClassAppInfo* info,
                                      const DateTime& startDate = DateTime::emptyDate(),
                                      const DateTime& endDate = DateTime::emptyDate());

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserterWithDateRange<FareClassAppInfo, FareClassAppHistoricalDAO>(
               flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  virtual sfc::CompressedData*
  compress(const std::vector<const FareClassAppInfo*>* vect) const override;

  virtual std::vector<const FareClassAppInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareClassAppHistoricalDAO>;
  static DAOHelper<FareClassAppHistoricalDAO> _helper;
  FareClassAppHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FareClassAppHistoricalKey, std::vector<const FareClassAppInfo*> >(
          cacheSize, cacheType, 4)
  {
  }
  std::vector<const FareClassAppInfo*>* create(FareClassAppHistoricalKey key) override;
  void destroy(FareClassAppHistoricalKey key, std::vector<const FareClassAppInfo*>* t) override;
  void load() override;

private:
  struct isNotApplicable;
  struct isNotApplicableByTravelDT;
  struct isNotApplicableForFD;
  struct isNotApplicableForFDByTravelDT;
  static FareClassAppHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
