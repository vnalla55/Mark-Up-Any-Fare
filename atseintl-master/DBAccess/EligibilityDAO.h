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
class EligibilityInfo;
class DeleteList;

typedef HashKey<VendorCode, int> EligibilityKey;

class EligibilityDAO : public DataAccessObject<EligibilityKey, std::vector<const EligibilityInfo*> >
{
public:
  static EligibilityDAO& instance();
  const EligibilityInfo*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, EligibilityKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  EligibilityKey createKey(const EligibilityInfo* info);

  void translateKey(const EligibilityKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<EligibilityInfo, EligibilityDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }
  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData*
  compress(const std::vector<const EligibilityInfo*>* vect) const override;

  virtual std::vector<const EligibilityInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<EligibilityDAO>;
  static DAOHelper<EligibilityDAO> _helper;
  EligibilityDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<EligibilityKey, std::vector<const EligibilityInfo*> >(
          cacheSize, cacheType, 2)
  {
  }
  void load() override;
  std::vector<const EligibilityInfo*>* create(EligibilityKey key) override;
  void destroy(EligibilityKey key, std::vector<const EligibilityInfo*>* t) override;

private:
  static EligibilityDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: EligibilityHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> EligibilityHistoricalKey;

class EligibilityHistoricalDAO
    : public HistoricalDataAccessObject<EligibilityHistoricalKey,
                                        std::vector<const EligibilityInfo*> >
{
public:
  static EligibilityHistoricalDAO& instance();
  const EligibilityInfo*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, EligibilityHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._c);
    objectKey.getValue("ENDDATE", key._d);
    return key.initialized = objectKey.getValue("VENDOR", key._a)
                             && objectKey.getValue("ITEMNO", key._b);
  }

  bool translateKey(const ObjectKey& objectKey,
                    EligibilityHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void setKeyDateRange(EligibilityHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData*
  compress(const std::vector<const EligibilityInfo*>* vect) const override;

  virtual std::vector<const EligibilityInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<EligibilityHistoricalDAO>;
  static DAOHelper<EligibilityHistoricalDAO> _helper;
  EligibilityHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<EligibilityHistoricalKey, std::vector<const EligibilityInfo*> >(
          cacheSize, cacheType, 2)
  {
  }
  std::vector<const EligibilityInfo*>* create(EligibilityHistoricalKey key) override;
  void destroy(EligibilityHistoricalKey key, std::vector<const EligibilityInfo*>* t) override;

private:
  static EligibilityHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
