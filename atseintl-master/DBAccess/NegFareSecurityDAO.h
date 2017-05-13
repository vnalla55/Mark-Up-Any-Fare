//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
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
class NegFareSecurityInfo;
class DeleteList;

typedef HashKey<VendorCode, int> NegFareSecurityKey;

class NegFareSecurityDAO
    : public DataAccessObject<NegFareSecurityKey, std::vector<NegFareSecurityInfo*> >
{
public:
  static NegFareSecurityDAO& instance();
  const std::vector<NegFareSecurityInfo*>&
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, NegFareSecurityKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  NegFareSecurityKey createKey(NegFareSecurityInfo* info);

  void translateKey(const NegFareSecurityKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<NegFareSecurityInfo, NegFareSecurityDAO>(flatKey, objectKey)
        .success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData*
  compress(const std::vector<NegFareSecurityInfo*>* vect) const override;

  virtual std::vector<NegFareSecurityInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<NegFareSecurityDAO>;
  static DAOHelper<NegFareSecurityDAO> _helper;
  NegFareSecurityDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<NegFareSecurityKey, std::vector<NegFareSecurityInfo*> >(
          cacheSize, cacheType, 2)
  {
  }
  void load() override;
  std::vector<NegFareSecurityInfo*>* create(NegFareSecurityKey key) override;
  void destroy(NegFareSecurityKey key, std::vector<NegFareSecurityInfo*>* t) override;

private:
  static NegFareSecurityDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: NegFareSecurityHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> NegFareSecurityHistoricalKey;

class NegFareSecurityHistoricalDAO
    : public HistoricalDataAccessObject<NegFareSecurityHistoricalKey,
                                        std::vector<NegFareSecurityInfo*> >
{
public:
  static NegFareSecurityHistoricalDAO& instance();
  const std::vector<NegFareSecurityInfo*>&
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, NegFareSecurityHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b) &&
               objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    NegFareSecurityHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  NegFareSecurityHistoricalKey createKey(NegFareSecurityInfo* info,
                                         const DateTime& startDate = DateTime::emptyDate(),
                                         const DateTime& endDate = DateTime::emptyDate());

  void translateKey(const NegFareSecurityHistoricalKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
    objectKey.setValue("STARTDATE", key._c);
    objectKey.setValue("ENDDATE", key._d);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserterWithDateRange<NegFareSecurityInfo, NegFareSecurityHistoricalDAO>(
               flatKey, objectKey).success();
  }

  void setKeyDateRange(NegFareSecurityHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  virtual sfc::CompressedData*
  compress(const std::vector<NegFareSecurityInfo*>* vect) const override;

  virtual std::vector<NegFareSecurityInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<NegFareSecurityHistoricalDAO>;
  static DAOHelper<NegFareSecurityHistoricalDAO> _helper;
  NegFareSecurityHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<NegFareSecurityHistoricalKey, std::vector<NegFareSecurityInfo*> >(
          cacheSize, cacheType, 2)
  {
  }
  std::vector<NegFareSecurityInfo*>* create(NegFareSecurityHistoricalKey key) override;
  void destroy(NegFareSecurityHistoricalKey key, std::vector<NegFareSecurityInfo*>* t) override;
  void load() override;

private:
  static NegFareSecurityHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
