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
class StopoversInfo;
class DeleteList;

typedef HashKey<VendorCode, int> StopoversKey;

class StopoversDAO : public DataAccessObject<StopoversKey, std::vector<StopoversInfo*> >
{
public:
  static StopoversDAO& instance();
  const StopoversInfo*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, StopoversKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  StopoversKey createKey(StopoversInfo* info);

  void translateKey(const StopoversKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<StopoversInfo, StopoversDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<StopoversInfo*>* vect) const override;

  virtual std::vector<StopoversInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<StopoversDAO>;
  static DAOHelper<StopoversDAO> _helper;
  StopoversDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<StopoversKey, std::vector<StopoversInfo*> >(cacheSize, cacheType, 2)
  {
  }
  void load() override;
  std::vector<StopoversInfo*>* create(StopoversKey key) override;
  void destroy(StopoversKey key, std::vector<StopoversInfo*>* t) override;

private:
  static StopoversDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: StopoversHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> StopoversHistoricalKey;

class StopoversHistoricalDAO
    : public HistoricalDataAccessObject<StopoversHistoricalKey, std::vector<StopoversInfo*> >
{
public:
  static StopoversHistoricalDAO& instance();
  const StopoversInfo*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, StopoversHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b) &&
               objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    StopoversHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  StopoversHistoricalKey createKey(const StopoversInfo* info,
                                   const DateTime& startDate = DateTime::emptyDate(),
                                   const DateTime& endDate = DateTime::emptyDate());

  void translateKey(const StopoversHistoricalKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
    objectKey.setValue("STARTDATE", key._c);
    objectKey.setValue("ENDDATE", key._d);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserterWithDateRange<StopoversInfo, StopoversHistoricalDAO>(
               flatKey, objectKey).success();
  }

  void setKeyDateRange(StopoversHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  virtual sfc::CompressedData* compress(const std::vector<StopoversInfo*>* vect) const override;

  virtual std::vector<StopoversInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<StopoversHistoricalDAO>;
  static DAOHelper<StopoversHistoricalDAO> _helper;
  StopoversHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<StopoversHistoricalKey, std::vector<StopoversInfo*> >(cacheSize,
                                                                                       cacheType)
  {
  }
  std::vector<StopoversInfo*>* create(StopoversHistoricalKey key) override;
  void destroy(StopoversHistoricalKey key, std::vector<StopoversInfo*>* t) override;
  void load() override;

private:
  static StopoversHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
