//----------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//----------------------------------------------------------------------------
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
class DayTimeAppInfo;
class DeleteList;

typedef HashKey<VendorCode, int> DayTimeAppInfoKey;

class DayTimeAppInfoDAO : public DataAccessObject<DayTimeAppInfoKey, std::vector<DayTimeAppInfo*> >
{
public:
  static DayTimeAppInfoDAO& instance();
  const DayTimeAppInfo*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, DayTimeAppInfoKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  DayTimeAppInfoKey createKey(DayTimeAppInfo* info);

  void translateKey(const DayTimeAppInfoKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<DayTimeAppInfo, DayTimeAppInfoDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<DayTimeAppInfo*>* vect) const override;

  virtual std::vector<DayTimeAppInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<DayTimeAppInfoDAO>;
  static DAOHelper<DayTimeAppInfoDAO> _helper;
  DayTimeAppInfoDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<DayTimeAppInfoKey, std::vector<DayTimeAppInfo*> >(cacheSize, cacheType, 2)
  {
  }
  void load() override;
  std::vector<DayTimeAppInfo*>* create(DayTimeAppInfoKey key) override;
  void destroy(DayTimeAppInfoKey key, std::vector<DayTimeAppInfo*>* recs) override;

private:
  static DayTimeAppInfoDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class DayTimeAppInfoDAO

// --------------------------------------------------
// Historical DAO: DayTimeAppInfoHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> DayTimeAppInfoHistoricalKey;

class DayTimeAppInfoHistoricalDAO
    : public HistoricalDataAccessObject<DayTimeAppInfoHistoricalKey, std::vector<DayTimeAppInfo*> >
{
public:
  static DayTimeAppInfoHistoricalDAO& instance();
  const DayTimeAppInfo*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, DayTimeAppInfoHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._c);
    objectKey.getValue("ENDDATE", key._d);
    return key.initialized = objectKey.getValue("VENDOR", key._a)
                             && objectKey.getValue("ITEMNO", key._b);
  }

  bool translateKey(const ObjectKey& objectKey,
                    DayTimeAppInfoHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void setKeyDateRange(DayTimeAppInfoHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<DayTimeAppInfo*>* vect) const override;

  virtual std::vector<DayTimeAppInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<DayTimeAppInfoHistoricalDAO>;
  static DAOHelper<DayTimeAppInfoHistoricalDAO> _helper;
  DayTimeAppInfoHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<DayTimeAppInfoHistoricalKey, std::vector<DayTimeAppInfo*> >(
          cacheSize, cacheType, 2)
  {
  }
  std::vector<DayTimeAppInfo*>* create(DayTimeAppInfoHistoricalKey key) override;
  void destroy(DayTimeAppInfoHistoricalKey key, std::vector<DayTimeAppInfo*>* recs) override;

private:
  static DayTimeAppInfoHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class DayTimeAppInfoHistoricalDAO
} // namespace tse
