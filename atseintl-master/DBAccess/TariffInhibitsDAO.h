//-------------------------------------------------------------------------------
// Copyright 2004, 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/HistoricalDataAccessObject.h"

namespace tse
{
class TariffInhibits;
class DeleteList;

typedef HashKey<VendorCode, Indicator, CarrierCode, TariffNumber, TariffCode> TariffInhibitsKey;

class TariffInhibitsDAO
    : public DataAccessObject<TariffInhibitsKey, std::vector<TariffInhibits*>, false>
{
public:
  static TariffInhibitsDAO& instance();
  const Indicator getTariffInhibit(const VendorCode& vendor,
                                   const Indicator tariffCrossRefType,
                                   const CarrierCode& carrier,
                                   const TariffNumber& fareTariff,
                                   const TariffCode& ruleTariffCode,
                                   DeleteList& del);

  bool translateKey(const ObjectKey& objectKey, TariffInhibitsKey& key) const override
  {
    return key.initialized = objectKey.getValue("VENDOR", key._a) &&
                             objectKey.getValue("TARIFFCROSSREFTYPE", key._b) &&
                             objectKey.getValue("CARRIER", key._c) &&
                             objectKey.getValue("FARETARIFF", key._d) &&
                             objectKey.getValue("RULETARIFFCODE", key._e);
  }

  TariffInhibitsKey createKey(TariffInhibits* info);

  void translateKey(const TariffInhibitsKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("TARIFFCROSSREFTYPE", key._b);
    objectKey.setValue("CARRIER", key._c);
    objectKey.setValue("FARETARIFF", key._d);
    objectKey.setValue("RULETARIFFCODE", key._e);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<TariffInhibits, TariffInhibitsDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<TariffInhibits*>* vect) const override;

  virtual std::vector<TariffInhibits*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<TariffInhibitsDAO>;
  static DAOHelper<TariffInhibitsDAO> _helper;
  TariffInhibitsDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<TariffInhibitsKey, std::vector<TariffInhibits*>, false>(cacheSize, cacheType)
  {
  }
  std::vector<TariffInhibits*>* create(TariffInhibitsKey key) override;
  void destroy(TariffInhibitsKey key, std::vector<TariffInhibits*>* recs) override;
  virtual void load() override;

private:
  static TariffInhibitsDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: TariffInhibitsHistoricalDAO
// --------------------------------------------------
class TariffInhibitsHistoricalDAO
    : public HistoricalDataAccessObject<TariffInhibitsKey, std::vector<TariffInhibits*> >
{
public:
  static TariffInhibitsHistoricalDAO& instance();
  const Indicator getTariffInhibit(const VendorCode& vendor,
                                   const Indicator tariffCrossRefType,
                                   const CarrierCode& carrier,
                                   const TariffNumber& fareTariff,
                                   const TariffCode& ruleTariffCode,
                                   DeleteList& del,
                                   const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, TariffInhibitsKey& key) const override
  {
    return key.initialized = objectKey.getValue("VENDOR", key._a) &&
                             objectKey.getValue("TARIFFCROSSREFTYPE", key._b) &&
                             objectKey.getValue("CARRIER", key._c) &&
                             objectKey.getValue("FARETARIFF", key._d) &&
                             objectKey.getValue("RULETARIFFCODE", key._e);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<TariffInhibits*>* vect) const override;

  virtual std::vector<TariffInhibits*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<TariffInhibitsHistoricalDAO>;
  static DAOHelper<TariffInhibitsHistoricalDAO> _helper;
  TariffInhibitsHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<TariffInhibitsKey, std::vector<TariffInhibits*> >(cacheSize,
                                                                                   cacheType)
  {
  }
  std::vector<TariffInhibits*>* create(TariffInhibitsKey key) override;
  void destroy(TariffInhibitsKey key, std::vector<TariffInhibits*>* recs) override;
  virtual void load() override;

private:
  struct groupByKey;
  static TariffInhibitsHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
