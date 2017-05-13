//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOUtils.h"
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
class SvcFeesCurrencyInfo;
class DeleteList;

typedef HashKey<VendorCode, int> SvcFeesCurrencyKey;

class SvcFeesCurrencyDAO
    : public DataAccessObject<SvcFeesCurrencyKey, std::vector<SvcFeesCurrencyInfo*> >
{
public:
  static SvcFeesCurrencyDAO& instance();

  const std::vector<SvcFeesCurrencyInfo*>&
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, SvcFeesCurrencyKey& key) const override
  {
    return (key.initialized =
                (objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b)));
  }

  SvcFeesCurrencyKey createKey(SvcFeesCurrencyInfo* info);

  void translateKey(const SvcFeesCurrencyKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<SvcFeesCurrencyInfo, SvcFeesCurrencyDAO>(flatKey, objectKey)
        .success();
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  virtual sfc::CompressedData*
  compress(const std::vector<SvcFeesCurrencyInfo*>* vect) const override;

  virtual std::vector<SvcFeesCurrencyInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<SvcFeesCurrencyDAO>;
  static DAOHelper<SvcFeesCurrencyDAO> _helper;
  SvcFeesCurrencyDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<SvcFeesCurrencyKey, std::vector<SvcFeesCurrencyInfo*> >(
          cacheSize, cacheType, 2)
  {
  }
  std::vector<SvcFeesCurrencyInfo*>* create(SvcFeesCurrencyKey key) override;
  void destroy(SvcFeesCurrencyKey key, std::vector<SvcFeesCurrencyInfo*>* recs) override;

  void load() override;

private:
  static SvcFeesCurrencyDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

typedef HashKey<VendorCode, int, DateTime, DateTime> SvcFeesCurrencyHistoricalKey;

class SvcFeesCurrencyHistoricalDAO
    : public HistoricalDataAccessObject<SvcFeesCurrencyHistoricalKey,
                                        std::vector<SvcFeesCurrencyInfo*> >
{
public:
  static SvcFeesCurrencyHistoricalDAO& instance();

  const std::vector<SvcFeesCurrencyInfo*>&
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, SvcFeesCurrencyHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b) &&
               objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    SvcFeesCurrencyHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void setKeyDateRange(SvcFeesCurrencyHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<SvcFeesCurrencyHistoricalDAO>;
  static DAOHelper<SvcFeesCurrencyHistoricalDAO> _helper;
  SvcFeesCurrencyHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<SvcFeesCurrencyHistoricalKey, std::vector<SvcFeesCurrencyInfo*> >(
          cacheSize, cacheType, 2)
  {
  }
  std::vector<SvcFeesCurrencyInfo*>* create(SvcFeesCurrencyHistoricalKey key) override;
  void destroy(SvcFeesCurrencyHistoricalKey key, std::vector<SvcFeesCurrencyInfo*>* t) override;

  virtual sfc::CompressedData*
  compress(const std::vector<SvcFeesCurrencyInfo*>* vect) const override;

  virtual std::vector<SvcFeesCurrencyInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

private:
  static SvcFeesCurrencyHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
