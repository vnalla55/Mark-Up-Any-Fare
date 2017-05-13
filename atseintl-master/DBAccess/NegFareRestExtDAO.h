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
class NegFareRestExt;
class DeleteList;

typedef HashKey<VendorCode, int> NegFareRestExtKey;

class NegFareRestExtDAO : public DataAccessObject<NegFareRestExtKey, std::vector<NegFareRestExt*> >
{
public:
  static NegFareRestExtDAO& instance();
  const NegFareRestExt*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, NegFareRestExtKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  NegFareRestExtKey createKey(NegFareRestExt* info);

  void translateKey(const NegFareRestExtKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<NegFareRestExt, NegFareRestExtDAO>(flatKey, objectKey).success();
  }

  virtual sfc::CompressedData* compress(const std::vector<NegFareRestExt*>* vect) const override;

  virtual std::vector<NegFareRestExt*>*
  uncompress(const sfc::CompressedData& compressed) const override;

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<NegFareRestExtDAO>;
  static DAOHelper<NegFareRestExtDAO> _helper;
  NegFareRestExtDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<NegFareRestExtKey, std::vector<NegFareRestExt*> >(cacheSize, cacheType)
  {
  }
  void load() override;
  std::vector<NegFareRestExt*>* create(NegFareRestExtKey key) override;
  void destroy(NegFareRestExtKey key, std::vector<NegFareRestExt*>* t) override;

private:
  static NegFareRestExtDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: NegFareRestExtHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> NegFareRestExtHistoricalKey;

class NegFareRestExtHistoricalDAO
    : public HistoricalDataAccessObject<NegFareRestExtHistoricalKey, std::vector<NegFareRestExt*> >
{
public:
  static NegFareRestExtHistoricalDAO& instance();
  const NegFareRestExt*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, NegFareRestExtHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._c);
    objectKey.getValue("ENDDATE", key._d);
    return key.initialized = objectKey.getValue("VENDOR", key._a)
                             && objectKey.getValue("ITEMNO", key._b);
               
  }

  bool translateKey(const ObjectKey& objectKey,
                    NegFareRestExtHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void setKeyDateRange(NegFareRestExtHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  virtual sfc::CompressedData* compress(const std::vector<NegFareRestExt*>* vect) const override;

  virtual std::vector<NegFareRestExt*>*
  uncompress(const sfc::CompressedData& compressed) const override;

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<NegFareRestExtHistoricalDAO>;
  static DAOHelper<NegFareRestExtHistoricalDAO> _helper;
  NegFareRestExtHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<NegFareRestExtHistoricalKey, std::vector<NegFareRestExt*> >(
          cacheSize, cacheType)
  {
  }
  std::vector<NegFareRestExt*>* create(NegFareRestExtHistoricalKey key) override;
  void destroy(NegFareRestExtHistoricalKey key, std::vector<NegFareRestExt*>* t) override;

private:
  static NegFareRestExtHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
