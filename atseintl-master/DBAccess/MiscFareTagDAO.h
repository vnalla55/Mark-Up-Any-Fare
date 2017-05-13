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
class MiscFareTag;
class DeleteList;

typedef HashKey<VendorCode, int> MiscFareTagKey;

class MiscFareTagDAO : public DataAccessObject<MiscFareTagKey, std::vector<MiscFareTag*> >
{
public:
  static MiscFareTagDAO& instance();
  const MiscFareTag*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, MiscFareTagKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  MiscFareTagKey createKey(MiscFareTag* info);

  void translateKey(const MiscFareTagKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<MiscFareTag, MiscFareTagDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<MiscFareTag*>* vect) const override;

  virtual std::vector<MiscFareTag*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<MiscFareTagDAO>;
  static DAOHelper<MiscFareTagDAO> _helper;
  MiscFareTagDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<MiscFareTagKey, std::vector<MiscFareTag*> >(cacheSize, cacheType, 2)
  {
  }
  void load() override;
  std::vector<MiscFareTag*>* create(MiscFareTagKey key) override;
  void destroy(MiscFareTagKey key, std::vector<MiscFareTag*>* t) override;

private:
  static MiscFareTagDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: MiscFareTagHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> MiscFareTagHistoricalKey;

class MiscFareTagHistoricalDAO
    : public HistoricalDataAccessObject<MiscFareTagHistoricalKey, std::vector<MiscFareTag*> >
{
public:
  static MiscFareTagHistoricalDAO& instance();
  const MiscFareTag*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, MiscFareTagHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b) &&
               objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    MiscFareTagHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void setKeyDateRange(MiscFareTagHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<MiscFareTag*>* vect) const override;

  virtual std::vector<MiscFareTag*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<MiscFareTagHistoricalDAO>;
  static DAOHelper<MiscFareTagHistoricalDAO> _helper;
  MiscFareTagHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<MiscFareTagHistoricalKey, std::vector<MiscFareTag*> >(
          cacheSize, cacheType, 2)
  {
  }
  std::vector<MiscFareTag*>* create(MiscFareTagHistoricalKey key) override;
  void destroy(MiscFareTagHistoricalKey key, std::vector<MiscFareTag*>* t) override;

private:
  static MiscFareTagHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
