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
class MerchActivationInfo;

typedef HashKey<uint64_t, CarrierCode, PseudoCityCode> MerchActivationKey;
typedef HashKey<uint64_t, CarrierCode, PseudoCityCode, DateTime, DateTime>
MerchActivationHistoricalKey;

class MerchActivationDAO
    : public DataAccessObject<MerchActivationKey, std::vector<MerchActivationInfo*> >
{
public:
  friend class DAOHelper<MerchActivationDAO>;

  static MerchActivationDAO& instance();

  const std::vector<MerchActivationInfo*>& get(DeleteList& del,
                                               uint64_t productId,
                                               const CarrierCode& carrier,
                                               const PseudoCityCode& pseudoCity,
                                               const DateTime& date,
                                               const DateTime& ticketDate);

  MerchActivationKey createKey(MerchActivationInfo* info);

  bool translateKey(const ObjectKey& objectKey, MerchActivationKey& key) const override
  {
    return key.initialized = objectKey.getValue("PRODUCTID", key._a) &&
                             objectKey.getValue("CARRIER", key._b) &&
                             objectKey.getValue("PSEUDOCITY", key._c);
  }

  void translateKey(const MerchActivationKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("PRODUCTID", key._a);
    objectKey.setValue("CARRIER", key._b);
    objectKey.setValue("PSEUDOCITY", key._c);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<MerchActivationInfo, MerchActivationDAO>(flatKey, objectKey)
        .success();
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  virtual sfc::CompressedData*
  compress(const std::vector<MerchActivationInfo*>* vect) const override;

  virtual std::vector<MerchActivationInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

private:
  MerchActivationDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<MerchActivationKey, std::vector<MerchActivationInfo*> >(cacheSize, cacheType)
  {
  }

  std::vector<MerchActivationInfo*>* create(MerchActivationKey key) override;
  void destroy(MerchActivationKey key, std::vector<MerchActivationInfo*>* recs) override;
  void load() override;

  static std::string _name;
  static std::string _cacheClass;
  static DAOHelper<MerchActivationDAO> _helper;
  static MerchActivationDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

class MerchActivationHistoricalDAO
    : public HistoricalDataAccessObject<MerchActivationHistoricalKey,
                                        std::vector<MerchActivationInfo*> >
{
public:
  friend class DAOHelper<MerchActivationHistoricalDAO>;

  static MerchActivationHistoricalDAO& instance();

  const std::vector<MerchActivationInfo*>& get(DeleteList& del,
                                               uint64_t productId,
                                               const CarrierCode& carrier,
                                               const PseudoCityCode& pseudoCity,
                                               const DateTime& date,
                                               const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, MerchActivationHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("PRODUCTID", key._a) && objectKey.getValue("CARRIER", key._b) &&
               objectKey.getValue("PSEUDOCITY", key._c) &&
               objectKey.getValue("STARTDATE", key._d) && objectKey.getValue("ENDDATE", key._e);
  }

  bool translateKey(const ObjectKey& objectKey,
                    MerchActivationHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);

    return key.initialized = objectKey.getValue("PRODUCTID", key._a) &&
                             objectKey.getValue("CARRIER", key._b) &&
                             objectKey.getValue("PSEUDOCITY", key._c);
  }

  void setKeyDateRange(MerchActivationHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  virtual sfc::CompressedData*
  compress(const std::vector<MerchActivationInfo*>* vect) const override;

  virtual std::vector<MerchActivationInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

private:
  MerchActivationHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<MerchActivationHistoricalKey, std::vector<MerchActivationInfo*> >(
          cacheSize, cacheType)
  {
  }
  std::vector<MerchActivationInfo*>* create(MerchActivationHistoricalKey key) override;
  void destroy(MerchActivationHistoricalKey key, std::vector<MerchActivationInfo*>* t) override;

  static std::string _name;
  static std::string _cacheClass;
  static DAOHelper<MerchActivationHistoricalDAO> _helper;
  static MerchActivationHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // tse

