//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
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
class SurchargesInfo;
class DeleteList;

typedef HashKey<VendorCode, int> SurchargesKey;

class SurchargesDAO : public DataAccessObject<SurchargesKey, std::vector<SurchargesInfo*> >
{
public:
  static SurchargesDAO& instance();
  const SurchargesInfo*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, SurchargesKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  SurchargesKey createKey(SurchargesInfo* info);

  void translateKey(const SurchargesKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<SurchargesInfo, SurchargesDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<SurchargesInfo*>* vect) const override;

  virtual std::vector<SurchargesInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<SurchargesDAO>;
  static DAOHelper<SurchargesDAO> _helper;
  SurchargesDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<SurchargesKey, std::vector<SurchargesInfo*> >(cacheSize, cacheType, 3)
  {
  }
  void load() override;
  std::vector<SurchargesInfo*>* create(SurchargesKey key) override;
  void destroy(SurchargesKey key, std::vector<SurchargesInfo*>* t) override;

private:
  static SurchargesDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: SurchargesHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> SurchargesHistoricalKey;

class SurchargesHistoricalDAO
    : public HistoricalDataAccessObject<SurchargesHistoricalKey, std::vector<SurchargesInfo*> >
{
public:
  static SurchargesHistoricalDAO& instance();
  const SurchargesInfo*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, SurchargesHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b) &&
               objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    SurchargesHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void setKeyDateRange(SurchargesHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  SurchargesHistoricalKey createKey(const SurchargesInfo* info,
                                    const DateTime& startDate = DateTime::emptyDate(),
                                    const DateTime& endDate = DateTime::emptyDate());

  void translateKey(const SurchargesHistoricalKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
    objectKey.setValue("STARTDATE", key._c);
    objectKey.setValue("ENDDATE", key._d);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserterWithDateRange<SurchargesInfo, SurchargesHistoricalDAO>(
               flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  virtual sfc::CompressedData* compress(const std::vector<SurchargesInfo*>* vect) const override;

  virtual std::vector<SurchargesInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<SurchargesHistoricalDAO>;
  static DAOHelper<SurchargesHistoricalDAO> _helper;
  SurchargesHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<SurchargesHistoricalKey, std::vector<SurchargesInfo*> >(
          cacheSize, cacheType, 3)
  {
  }
  std::vector<SurchargesInfo*>* create(SurchargesHistoricalKey key) override;
  void destroy(SurchargesHistoricalKey key, std::vector<SurchargesInfo*>* t) override;
  void load() override;

private:
  static SurchargesHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
