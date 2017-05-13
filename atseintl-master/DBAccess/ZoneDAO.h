//-------------------------------------------------------------------------------
// Copyright 2004, 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/HistoricalDataAccessObject.h"

namespace tse
{
class ZoneInfo;
class DeleteList;

typedef HashKey<VendorCode, Zone, Indicator> ZoneKey;

class ZoneDAO : public DataAccessObject<ZoneKey, std::vector<const ZoneInfo*> >
{
public:
  static ZoneDAO& instance();
  static log4cxx::LoggerPtr _logger;

  const ZoneInfo* get(DeleteList& del,
                      const VendorCode& vendor,
                      const Zone& zone,
                      Indicator zoneType,
                      const DateTime& date,
                      const DateTime& ticketDate,
                      bool fareFocusGroup = false);

  bool translateKey(const ObjectKey& objectKey, ZoneKey& key) const override
  {
    return key.initialized = objectKey.getValue("VENDOR", key._a) &&
                             objectKey.getValue("ZONENO", key._b) &&
                             objectKey.getValue("ZONETYPE", key._c);
  }

  ZoneKey createKey(const ZoneInfo* info);

  void translateKey(const ZoneKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ZONENO", key._b);
    objectKey.setValue("ZONETYPE", key._c);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<ZoneInfo, ZoneDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<const ZoneInfo*>* vect) const override;

  virtual std::vector<const ZoneInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<ZoneDAO>;

  static DAOHelper<ZoneDAO> _helper;

  ZoneDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<ZoneKey, std::vector<const ZoneInfo*> >(cacheSize, cacheType, 3)
  {
  }

  void load() override;

  std::vector<const ZoneInfo*>* create(ZoneKey key) override;

  void destroy(ZoneKey key, std::vector<const ZoneInfo*>* t) override;

private:
  struct isEffective;

  static ZoneDAO* _instance;

}; // class ZoneDAO

// --------------------------------------------------
// Historical DAO: ZoneHistoricalDAO
// --------------------------------------------------

typedef HashKey<VendorCode, Zone, Indicator, DateTime, DateTime> ZoneHistoricalKey;

class ZoneHistoricalDAO
    : public HistoricalDataAccessObject<ZoneHistoricalKey, std::vector<const ZoneInfo*> >
{
public:
  static ZoneHistoricalDAO& instance();
  static log4cxx::LoggerPtr _logger;

  const ZoneInfo* get(DeleteList& del,
                      const VendorCode& vendor,
                      const Zone& zone,
                      Indicator zoneType,
                      const DateTime& date,
                      const DateTime& ticketDate,
                      bool fareFocusGroup = false);

  bool translateKey(const ObjectKey& objectKey, ZoneHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ZONENO", key._b) &&
               objectKey.getValue("ZONETYPE", key._c) && objectKey.getValue("STARTDATE", key._d) &&
               objectKey.getValue("ENDDATE", key._e);
  }

  bool
  translateKey(const ObjectKey& objectKey, ZoneHistoricalKey& key, const DateTime ticketDate) const
      override
  {
    DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
    return key.initialized = objectKey.getValue("VENDOR", key._a) &&
                             objectKey.getValue("ZONENO", key._b) &&
                             objectKey.getValue("ZONETYPE", key._c);
  }

  ZoneHistoricalKey createKey(const ZoneInfo* info,
                              const DateTime& startDate = DateTime::emptyDate(),
                              const DateTime& endDate = DateTime::emptyDate());

  void translateKey(const ZoneHistoricalKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ZONENO", key._b);
    objectKey.setValue("ZONETYPE", key._c);
    objectKey.setValue("STARTDATE", key._d);
    objectKey.setValue("ENDDATE", key._e);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserterWithDateRange<ZoneInfo, ZoneHistoricalDAO>(flatKey, objectKey)
        .success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  void setKeyDateRange(ZoneHistoricalKey& key, const DateTime date) const override
  {
    DAOUtils::getDateRange(date, key._d, key._e, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<const ZoneInfo*>* vect) const override;

  virtual std::vector<const ZoneInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<ZoneHistoricalDAO>;

  static DAOHelper<ZoneHistoricalDAO> _helper;

  ZoneHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<ZoneHistoricalKey, std::vector<const ZoneInfo*> >(
          cacheSize, cacheType, 3)
  {
  }

  std::vector<const ZoneInfo*>* create(ZoneHistoricalKey key) override;

  void destroy(ZoneHistoricalKey key, std::vector<const ZoneInfo*>* t) override;
  void load() override;

private:
  struct isEffective;
  static ZoneHistoricalDAO* _instance;

}; // class ZoneHistoricalDAO

} // namespace tse

