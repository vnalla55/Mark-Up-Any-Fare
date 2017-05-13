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

typedef HashKey<CarrierCode, AccountCode> FareByRuleAppKey;

class FareByRuleAppDAO : public DataAccessObject<FareByRuleAppKey, std::vector<FareByRuleApp*> >
{
public:
  static FareByRuleAppDAO& instance();

  // find all FareByRuleApp that match an account code and we don't have a CorpId
  const std::vector<FareByRuleApp*>& get(DeleteList& del,
                                         const CarrierCode& carrier,
                                         const AccountCode& accountCode,
                                         const TktDesignator& tktDesignator,
                                         const std::vector<PaxTypeCode>& paxTypes,
                                         const DateTime& tvlDate,
                                         const DateTime& ticketDate,
                                         bool isFareDisplay);

  // find all FareByRuleApp that match an account code from a CorpId
  const std::vector<FareByRuleApp*>& get(DeleteList& del,
                                         const CarrierCode& carrier,
                                         const AccountCode& accountCode,
                                         const TktDesignator& tktDesignator,
                                         const std::vector<PaxTypeCode>& paxTypes,
                                         const std::vector<CorpId*>& corpIds,
                                         const DateTime& tvlDate,
                                         const DateTime& ticketDate,
                                         bool isFareDisplay);

  // find all FareByRuleApp with no account code that match CorpIds
  // (exclude those that match with CorpIds with no account code,
  // those are retrieved by rule tariff)
  const std::vector<FareByRuleApp*>& get(DeleteList& del,
                                         const CarrierCode& carrier,
                                         const TktDesignator& tktDesignator,
                                         const std::vector<PaxTypeCode>& paxTypes,
                                         const std::vector<CorpId*>& corpIds,
                                         const DateTime& tvlDate,
                                         const DateTime& ticketDate,
                                         bool isFareDisplay);

  // find all FareByRuleApp with no account code and adult pax type
  const std::vector<FareByRuleApp*>& get(DeleteList& del,
                                         const CarrierCode& carrier,
                                         const TktDesignator& tktDesignator,
                                         const std::vector<PaxTypeCode>& paxTypes,
                                         const DateTime& tvlDate,
                                         const DateTime& ticketDate,
                                         bool isFareDisplay);

  bool translateKey(const ObjectKey& objectKey, FareByRuleAppKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("CARRIER", key._a) && objectKey.getValue("ACCOUNTCODE", key._b);
  }

  FareByRuleAppKey createKey(const FareByRuleApp* info);

  void translateKey(const FareByRuleAppKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CARRIER", key._a);
    objectKey.setValue("ACCOUNTCODE", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<FareByRuleApp, FareByRuleAppDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<FareByRuleApp*>* vect) const override;

  virtual std::vector<FareByRuleApp*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareByRuleAppDAO>;
  static DAOHelper<FareByRuleAppDAO> _helper;
  FareByRuleAppDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FareByRuleAppKey, std::vector<FareByRuleApp*> >(cacheSize, cacheType, 4)
  {
  }
  std::vector<FareByRuleApp*>* create(FareByRuleAppKey key) override;
  void destroy(FareByRuleAppKey key, std::vector<FareByRuleApp*>* recs) override;
  void load() override;

private:
  struct FilterPaxType;
  struct FilterCorpId;
  struct FilterAdult;
  static FareByRuleAppDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

typedef HashKey<CarrierCode, AccountCode, DateTime, DateTime> FareByRuleAppHistoricalKey;

class FareByRuleAppHistoricalDAO
    : public HistoricalDataAccessObject<FareByRuleAppHistoricalKey, std::vector<FareByRuleApp*> >
{
public:
  static FareByRuleAppHistoricalDAO& instance();

  // find all FareByRuleApp that match an account code and we don't have a CorpId
  const std::vector<FareByRuleApp*>& get(DeleteList& del,
                                         const CarrierCode& carrier,
                                         const AccountCode& accountCode,
                                         const TktDesignator& tktDesignator,
                                         const std::vector<PaxTypeCode>& paxTypes,
                                         const DateTime& tvlDate,
                                         const DateTime& ticketDate,
                                         bool isFareDisplay);

  // find all FareByRuleApp that match an account code from a CorpId
  const std::vector<FareByRuleApp*>& get(DeleteList& del,
                                         const CarrierCode& carrier,
                                         const AccountCode& accountCode,
                                         const TktDesignator& tktDesignator,
                                         const std::vector<PaxTypeCode>& paxTypes,
                                         const std::vector<CorpId*>& corpIds,
                                         const DateTime& tvlDate,
                                         const DateTime& ticketDate,
                                         bool isFareDisplay);

  // find all FareByRuleApp with no account code that match CorpIds
  // (exclude those that match with CorpIds with no account code,
  // those are retrieved by rule tariff)
  const std::vector<FareByRuleApp*>& get(DeleteList& del,
                                         const CarrierCode& carrier,
                                         const TktDesignator& tktDesignator,
                                         const std::vector<PaxTypeCode>& paxTypes,
                                         const std::vector<CorpId*>& corpIds,
                                         const DateTime& tvlDate,
                                         const DateTime& ticketDate,
                                         bool isFareDisplay);

  // find all FareByRuleApp with no account code and adult pax type
  const std::vector<FareByRuleApp*>& get(DeleteList& del,
                                         const CarrierCode& carrier,
                                         const TktDesignator& tktDesignator,
                                         const std::vector<PaxTypeCode>& paxTypes,
                                         const DateTime& tvlDate,
                                         const DateTime& ticketDate,
                                         bool isFareDisplay);

  bool translateKey(const ObjectKey& objectKey, FareByRuleAppHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._c);
    objectKey.getValue("ENDDATE", key._d);
    return key.initialized = objectKey.getValue("CARRIER", key._a)
                             && objectKey.getValue("ACCOUNTCODE", key._b);
  }

  bool translateKey(const ObjectKey& objectKey,
                    FareByRuleAppHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("CARRIER", key._a) && objectKey.getValue("ACCOUNTCODE", key._b);
  }

  void setKeyDateRange(FareByRuleAppHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  FareByRuleAppHistoricalKey createKey(const FareByRuleApp* info,
                                       const DateTime& startDate = DateTime::emptyDate(),
                                       const DateTime& endDate = DateTime::emptyDate());

  void translateKey(const FareByRuleAppHistoricalKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CARRIER", key._a);
    objectKey.setValue("ACCOUNTCODE", key._b);
    objectKey.setValue("STARTDATE", key._c);
    objectKey.setValue("ENDDATE", key._d);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserterWithDateRange<FareByRuleApp, FareByRuleAppHistoricalDAO>(
               flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  virtual sfc::CompressedData* compress(const std::vector<FareByRuleApp*>* vect) const override;

  virtual std::vector<FareByRuleApp*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareByRuleAppHistoricalDAO>;
  static DAOHelper<FareByRuleAppHistoricalDAO> _helper;
  FareByRuleAppHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FareByRuleAppHistoricalKey, std::vector<FareByRuleApp*> >(
          cacheSize, cacheType, 4)
  {
  }
  std::vector<FareByRuleApp*>* create(FareByRuleAppHistoricalKey key) override;
  void destroy(FareByRuleAppHistoricalKey key, std::vector<FareByRuleApp*>* recs) override;
  void load() override;

private:
  struct FilterPaxType;
  struct FilterCorpId;
  struct FilterAdult;
  static FareByRuleAppHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
