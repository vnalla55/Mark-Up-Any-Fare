//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
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
class FareProperties;

typedef HashKey<VendorCode, CarrierCode, TariffNumber, RuleNumber> FarePropertiesKey;
typedef HashKey<VendorCode, CarrierCode, TariffNumber, RuleNumber, DateTime, DateTime>
FarePropertiesHistoricalKey;

class FarePropertiesDAO : public DataAccessObject<FarePropertiesKey, std::vector<FareProperties*> >
{
public:
  friend class DAOHelper<FarePropertiesDAO>;

  static FarePropertiesDAO& instance();

  const FareProperties* get(DeleteList& del,
                            const VendorCode& vendor,
                            const CarrierCode& carrier,
                            const TariffNumber& tariff,
                            const RuleNumber& rule,
                            const DateTime& ticketDate);

  FarePropertiesKey createKey(FareProperties* info);

  bool translateKey(const ObjectKey& objectKey, FarePropertiesKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b) &&
               objectKey.getValue("RULETARIFF", key._c) && objectKey.getValue("RULE", key._d);
  }

  void translateKey(const FarePropertiesKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("CARRIER", key._b);
    objectKey.setValue("RULETARIFF", key._c);
    objectKey.setValue("RULE", key._d);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<FareProperties, FarePropertiesDAO>(flatKey, objectKey).success();
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  virtual sfc::CompressedData*
    compress(const std::vector<FareProperties*>* vect) const override;

  virtual std::vector<FareProperties*>*
    uncompress(const sfc::CompressedData& compressed) const override;

private:
  FarePropertiesDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FarePropertiesKey, std::vector<FareProperties*> >(cacheSize, cacheType, 1)
  {
  }

  std::vector<FareProperties*>* create(FarePropertiesKey key) override;
  void destroy(FarePropertiesKey key, std::vector<FareProperties*>* recs) override;
  void load() override;

  static std::string _name;
  static std::string _cacheClass;
  static DAOHelper<FarePropertiesDAO> _helper;
  static FarePropertiesDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

class FarePropertiesHistoricalDAO
    : public HistoricalDataAccessObject<FarePropertiesHistoricalKey, std::vector<FareProperties*> >
{
public:
  friend class DAOHelper<FarePropertiesHistoricalDAO>;

  static FarePropertiesHistoricalDAO& instance();

  const FareProperties* get(DeleteList& del,
                            const VendorCode& vendor,
                            const CarrierCode& carrier,
                            const TariffNumber& tariff,
                            const RuleNumber& rule,
                            const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, FarePropertiesHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._e);
    objectKey.getValue("ENDDATE", key._f);
    return key.initialized = objectKey.getValue("VENDOR", key._a)
                             && objectKey.getValue("CARRIER", key._b)
                             && objectKey.getValue("RULETARIFF", key._c)
                             && objectKey.getValue("RULE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    FarePropertiesHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._e, key._f, _cacheBy);

    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b) &&
               objectKey.getValue("RULETARIFF", key._c) && objectKey.getValue("RULE", key._d);
  }

  void setKeyDateRange(FarePropertiesHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._e, key._f, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

private:
  FarePropertiesHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FarePropertiesHistoricalKey, std::vector<FareProperties*> >(
          cacheSize, cacheType, 1)
  {
  }
  std::vector<FareProperties*>* create(FarePropertiesHistoricalKey key) override;
  void destroy(FarePropertiesHistoricalKey key, std::vector<FareProperties*>* t) override;

  virtual sfc::CompressedData*
    compress(const std::vector<FareProperties*>* vect) const override;

  virtual std::vector<FareProperties*>*
    uncompress(const sfc::CompressedData& compressed) const override;

  static std::string _name;
  static std::string _cacheClass;
  static DAOHelper<FarePropertiesHistoricalDAO> _helper;
  static FarePropertiesHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // tse

