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
#include "DBAccess/DAOUtils.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/HistoricalDataAccessObject.h"

namespace tse
{
class FareInfo;
class DeleteList;

typedef HashKey<LocCode, LocCode, CarrierCode> FareKey;

class FareDAO : public DataAccessObject<FareKey, std::vector<const FareInfo*> >
{
public:
  static FareDAO& instance();

  const std::vector<const FareInfo*>& get(DeleteList& del,
                                          const LocCode& market1,
                                          const LocCode& market2,
                                          const CarrierCode& cxr,
                                          const DateTime& startDate,
                                          const DateTime& endDate,
                                          const DateTime& ticketDate,
                                          bool fareDisplay = false);

  const std::vector<const FareInfo*>& get(DeleteList& del,
                                          const LocCode& market1,
                                          const LocCode& market2,
                                          const CarrierCode& cxr,
                                          const VendorCode& vendor,
                                          const DateTime& ticketDate);

  void loadFaresForMarket(const LocCode& market1,
                          const LocCode& market2,
                          const std::vector<CarrierCode>& cxr);

  bool isRuleInFareMarket(const LocCode& market1,
                          const LocCode& market2,
                          const CarrierCode& cxr,
                          const RuleNumber ruleNumber);

  bool translateKey(const ObjectKey& objectKey, FareKey& key) const override
  {
    return key.initialized = objectKey.getValue("MARKET1", key._a) &&
                             objectKey.getValue("MARKET2", key._b) &&
                             objectKey.getValue("CARRIER", key._c);
  }

  FareKey createKey(const FareInfo* info);

  void translateKey(const FareKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("MARKET1", key._a);
    objectKey.setValue("MARKET2", key._b);
    objectKey.setValue("CARRIER", key._c);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<FareInfo, FareDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<const FareInfo*>* vect) const override;

  virtual std::vector<const FareInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<FareDAO>;

  static DAOHelper<FareDAO> _helper;

  FareDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FareKey, std::vector<const FareInfo*> >(cacheSize, cacheType, 7)
  {
  }

  std::vector<const FareInfo*>* create(FareKey key) override;

  void destroy(FareKey key, std::vector<const FareInfo*>* t) override;

  virtual void load() override;

private:
  struct isEffective;
  struct isVendor;
  static FareDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: FareHistoricalDAO
// --------------------------------------------------

typedef HashKey<LocCode, LocCode, CarrierCode, DateTime, DateTime> FareHistoricalKey;

class FareHistoricalDAO
    : public HistoricalDataAccessObject<FareHistoricalKey, std::vector<const FareInfo*> >
{
public:
  static FareHistoricalDAO& instance();

  const std::vector<const FareInfo*>& get(DeleteList& del,
                                          const LocCode& market1,
                                          const LocCode& market2,
                                          const CarrierCode& cxr,
                                          const DateTime& startDate,
                                          const DateTime& endDate,
                                          const DateTime& ticketDate,
                                          bool fareDisplay = false);

  const std::vector<const FareInfo*>& get(DeleteList& del,
                                          const LocCode& market1,
                                          const LocCode& market2,
                                          const CarrierCode& cxr,
                                          const VendorCode& vendor,
                                          const DateTime& ticketDate);

  void loadFaresForMarket(const LocCode& market1,
                          const LocCode& market2,
                          const std::vector<CarrierCode>& cxr,
                          const DateTime& ticketDate);

  bool isRuleInFareMarket(const LocCode& market1,
                          const LocCode& market2,
                          const CarrierCode& cxr,
                          const RuleNumber ruleNumber,
                          const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._d);
    objectKey.getValue("ENDDATE", key._e);
    return key.initialized = objectKey.getValue("MARKET1", key._a)
                             && objectKey.getValue("MARKET2", key._b)
                             && objectKey.getValue("CARRIER", key._c);
  }

  bool
  translateKey(const ObjectKey& objectKey, FareHistoricalKey& key, const DateTime ticketDate) const
      override
  {
    DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
    return key.initialized = objectKey.getValue("MARKET1", key._a) &&
                             objectKey.getValue("MARKET2", key._b) &&
                             objectKey.getValue("CARRIER", key._c);
  }

  FareHistoricalKey createKey(const FareInfo* info,
                              const DateTime& startDate = DateTime::emptyDate(),
                              const DateTime& endDate = DateTime::emptyDate());

  void translateKey(const FareHistoricalKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("MARKET1", key._a);
    objectKey.setValue("MARKET2", key._b);
    objectKey.setValue("CARRIER", key._c);
    objectKey.setValue("STARTDATE", key._d);
    objectKey.setValue("ENDDATE", key._e);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<FareInfo, FareHistoricalDAO>(flatKey, objectKey).success();
  }

  void setKeyDateRange(FareHistoricalKey& key, const DateTime date) const override
  {
    DAOUtils::getDateRange(date, key._d, key._e, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  virtual sfc::CompressedData* compress(const std::vector<const FareInfo*>* vect) const override;

  virtual std::vector<const FareInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<FareHistoricalDAO>;

  static DAOHelper<FareHistoricalDAO> _helper;

  FareHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FareHistoricalKey, std::vector<const FareInfo*> >(
          cacheSize, cacheType, 6)
  {
  }

  std::vector<const FareInfo*>* create(FareHistoricalKey key) override;
  void destroy(FareHistoricalKey key, std::vector<const FareInfo*>* t) override;

  void load() override;

private:
  struct isEffective;
  struct isVendor;
  static FareHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

} // namespace tse
