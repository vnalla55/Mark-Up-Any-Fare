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
class MerchCarrierPreferenceInfo;

typedef HashKey<CarrierCode, ServiceGroup> MerchCarrierPreferenceKey;
typedef HashKey<CarrierCode, ServiceGroup, DateTime, DateTime> MerchCarrierPreferenceHistoricalKey;

class MerchCarrierPreferenceDAO
    : public DataAccessObject<MerchCarrierPreferenceKey, std::vector<MerchCarrierPreferenceInfo*> >
{
public:
  friend class DAOHelper<MerchCarrierPreferenceDAO>;

  static MerchCarrierPreferenceDAO& instance();

  const MerchCarrierPreferenceInfo* get(DeleteList& del,
                                        const CarrierCode& carrier,
                                        const ServiceGroup& groupCode,
                                        const DateTime& ticketDate);

  MerchCarrierPreferenceKey createKey(MerchCarrierPreferenceInfo* info);

  bool translateKey(const ObjectKey& objectKey, MerchCarrierPreferenceKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("CARRIER", key._a) && objectKey.getValue("SVCGROUP", key._b);
  }

  void translateKey(const MerchCarrierPreferenceKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CARRIER", key._a);
    objectKey.setValue("SVCGROUP", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<MerchCarrierPreferenceInfo, MerchCarrierPreferenceDAO>(
               flatKey, objectKey).success();
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  virtual sfc::CompressedData*
  compress(const std::vector<MerchCarrierPreferenceInfo*>* vect) const override;

  virtual std::vector<MerchCarrierPreferenceInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

private:
  MerchCarrierPreferenceDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<MerchCarrierPreferenceKey, std::vector<MerchCarrierPreferenceInfo*> >(
          cacheSize, cacheType)
  {
  }

  std::vector<MerchCarrierPreferenceInfo*>* create(MerchCarrierPreferenceKey key) override;
  void
  destroy(MerchCarrierPreferenceKey key, std::vector<MerchCarrierPreferenceInfo*>* recs) override;
  void load() override;

  static std::string _name;
  static std::string _cacheClass;
  static DAOHelper<MerchCarrierPreferenceDAO> _helper;
  static MerchCarrierPreferenceDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

class MerchCarrierPreferenceHistoricalDAO
    : public HistoricalDataAccessObject<MerchCarrierPreferenceHistoricalKey,
                                        std::vector<MerchCarrierPreferenceInfo*> >
{
public:
  friend class DAOHelper<MerchCarrierPreferenceHistoricalDAO>;

  static MerchCarrierPreferenceHistoricalDAO& instance();

  const MerchCarrierPreferenceInfo* get(DeleteList& del,
                                        const CarrierCode& carrier,
                                        const ServiceGroup& groupCode,
                                        const DateTime& ticketDate);

  bool
  translateKey(const ObjectKey& objectKey, MerchCarrierPreferenceHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("CARRIER", key._a) && objectKey.getValue("SVCGROUP", key._b) &&
               objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    MerchCarrierPreferenceHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);

    return key.initialized =
               objectKey.getValue("CARRIER", key._a) && objectKey.getValue("SVCGROUP", key._b);
  }

  void setKeyDateRange(MerchCarrierPreferenceHistoricalKey& key, const DateTime ticketDate) const
      override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  virtual sfc::CompressedData*
  compress(const std::vector<MerchCarrierPreferenceInfo*>* vect) const override;

  virtual std::vector<MerchCarrierPreferenceInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

private:
  MerchCarrierPreferenceHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<MerchCarrierPreferenceHistoricalKey,
                                 std::vector<MerchCarrierPreferenceInfo*> >(cacheSize, cacheType)
  {
  }
  std::vector<MerchCarrierPreferenceInfo*>*
  create(MerchCarrierPreferenceHistoricalKey key) override;
  void destroy(MerchCarrierPreferenceHistoricalKey key,
               std::vector<MerchCarrierPreferenceInfo*>* t) override;

  static std::string _name;
  static std::string _cacheClass;
  static DAOHelper<MerchCarrierPreferenceHistoricalDAO> _helper;
  static MerchCarrierPreferenceHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // tse

