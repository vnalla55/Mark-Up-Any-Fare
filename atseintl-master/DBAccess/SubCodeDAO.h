//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
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
class SubCodeInfo;

typedef HashKey<VendorCode, CarrierCode> SubCodeKey;
typedef HashKey<VendorCode, CarrierCode, DateTime, DateTime> SubCodeHistoricalKey;

class SubCodeDAO : public DataAccessObject<SubCodeKey, std::vector<SubCodeInfo*> >
{
public:
  friend class DAOHelper<SubCodeDAO>;

  static SubCodeDAO& instance();

  const std::vector<SubCodeInfo*>& get(DeleteList& del,
                                       const VendorCode& vendor,
                                       const CarrierCode& carrier,
                                       const ServiceTypeCode& serviceTypeCode,
                                       const ServiceGroup& serviceGroup,
                                       const DateTime& date,
                                       const DateTime& ticketDate);

  SubCodeKey createKey(SubCodeInfo* info);

  bool translateKey(const ObjectKey& objectKey, SubCodeKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b);
  }

  void translateKey(const SubCodeKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("CARRIER", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<SubCodeInfo, SubCodeDAO>(flatKey, objectKey).success();
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  virtual sfc::CompressedData* compress(const std::vector<SubCodeInfo*>* vect) const override;

  virtual std::vector<SubCodeInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

private:
  SubCodeDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<SubCodeKey, std::vector<SubCodeInfo*> >(cacheSize, cacheType, 3)
  {
  }

  std::vector<SubCodeInfo*>* create(SubCodeKey key) override;
  void destroy(SubCodeKey key, std::vector<SubCodeInfo*>* recs) override;
  void load() override;

  static std::string _name;
  static std::string _cacheClass;
  static DAOHelper<SubCodeDAO> _helper;
  static SubCodeDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

class SubCodeHistoricalDAO
    : public HistoricalDataAccessObject<SubCodeHistoricalKey, std::vector<SubCodeInfo*> >
{
public:
  friend class DAOHelper<SubCodeHistoricalDAO>;

  static SubCodeHistoricalDAO& instance();

  const std::vector<SubCodeInfo*>& get(DeleteList& del,
                                       const VendorCode& vendor,
                                       const CarrierCode& carrier,
                                       const ServiceTypeCode& serviceTypeCode,
                                       const ServiceGroup& serviceGroup,
                                       const DateTime& date,
                                       const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, SubCodeHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b) &&
               objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    SubCodeHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);

    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b);
  }

  void setKeyDateRange(SubCodeHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  virtual sfc::CompressedData* compress(const std::vector<SubCodeInfo*>* vect) const override;

  virtual std::vector<SubCodeInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

private:
  SubCodeHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<SubCodeHistoricalKey, std::vector<SubCodeInfo*> >(
          cacheSize, cacheType, 3)
  {
  }
  std::vector<SubCodeInfo*>* create(SubCodeHistoricalKey key) override;
  void destroy(SubCodeHistoricalKey key, std::vector<SubCodeInfo*>* t) override;

  static std::string _name;
  static std::string _cacheClass;
  static DAOHelper<SubCodeHistoricalDAO> _helper;
  static SubCodeHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // tse

