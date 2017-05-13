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
class NegFareRest;
class DeleteList;

typedef HashKey<VendorCode, int> NegFareRestKey;

class NegFareRestDAO : public DataAccessObject<NegFareRestKey, std::vector<NegFareRest*> >
{
public:
  static NegFareRestDAO& instance();
  const NegFareRest*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, NegFareRestKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  NegFareRestKey createKey(NegFareRest* info);

  void translateKey(const NegFareRestKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<NegFareRest, NegFareRestDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<NegFareRest*>* vect) const override;

  virtual std::vector<NegFareRest*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<NegFareRestDAO>;
  static DAOHelper<NegFareRestDAO> _helper;
  NegFareRestDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<NegFareRestKey, std::vector<NegFareRest*> >(cacheSize, cacheType, 3)
  {
  }
  void load() override;
  std::vector<NegFareRest*>* create(NegFareRestKey key) override;
  void destroy(NegFareRestKey key, std::vector<NegFareRest*>* t) override;

private:
  static NegFareRestDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: NegFareRestHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> NegFareRestHistoricalKey;

class NegFareRestHistoricalDAO
    : public HistoricalDataAccessObject<NegFareRestHistoricalKey, std::vector<NegFareRest*> >
{
public:
  static NegFareRestHistoricalDAO& instance();
  const NegFareRest*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, NegFareRestHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b) &&
               objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    NegFareRestHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void translateKey(const NegFareRestHistoricalKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
    objectKey.setValue("STARTDATE", key._c);
    objectKey.setValue("ENDDATE", key._d);
  }

  NegFareRestHistoricalKey createKey(NegFareRest* info,
                                     const DateTime& startDate = DateTime::emptyDate(),
                                     const DateTime& endDate = DateTime::emptyDate());

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserterWithDateRange<NegFareRest, NegFareRestHistoricalDAO>(
               flatKey, objectKey).success();
  }

  void setKeyDateRange(NegFareRestHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  virtual sfc::CompressedData* compress(const std::vector<NegFareRest*>* vect) const override;

  virtual std::vector<NegFareRest*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<NegFareRestHistoricalDAO>;
  static DAOHelper<NegFareRestHistoricalDAO> _helper;
  NegFareRestHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<NegFareRestHistoricalKey, std::vector<NegFareRest*> >(
          cacheSize, cacheType, 3)
  {
  }
  std::vector<NegFareRest*>* create(NegFareRestHistoricalKey key) override;
  void destroy(NegFareRestHistoricalKey key, std::vector<NegFareRest*>* t) override;
  void load() override;

private:
  static NegFareRestHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
