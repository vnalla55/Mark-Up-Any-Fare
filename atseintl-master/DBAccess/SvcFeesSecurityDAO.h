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
class SvcFeesSecurityInfo;
class DeleteList;

typedef HashKey<VendorCode, int> SvcFeesSecurityKey;

class SvcFeesSecurityDAO
    : public DataAccessObject<SvcFeesSecurityKey, std::vector<SvcFeesSecurityInfo*> >
{
public:
  static SvcFeesSecurityDAO& instance();

  const std::vector<SvcFeesSecurityInfo*>&
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, SvcFeesSecurityKey& key) const override
  {
    return (key.initialized =
                (objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b)));
  }

  SvcFeesSecurityKey createKey(SvcFeesSecurityInfo* info);

  void translateKey(const SvcFeesSecurityKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<SvcFeesSecurityInfo, SvcFeesSecurityDAO>(flatKey, objectKey)
        .success();
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  virtual sfc::CompressedData*
  compress(const std::vector<SvcFeesSecurityInfo*>* vect) const override;

  virtual std::vector<SvcFeesSecurityInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<SvcFeesSecurityDAO>;
  static DAOHelper<SvcFeesSecurityDAO> _helper;
  SvcFeesSecurityDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<SvcFeesSecurityKey, std::vector<SvcFeesSecurityInfo*> >(cacheSize, cacheType)
  {
  }
  std::vector<SvcFeesSecurityInfo*>* create(SvcFeesSecurityKey key) override;
  void destroy(SvcFeesSecurityKey key, std::vector<SvcFeesSecurityInfo*>* recs) override;

  void load() override;

private:
  static SvcFeesSecurityDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

typedef HashKey<VendorCode, int, DateTime, DateTime> SvcFeesSecurityHistoricalKey;

class SvcFeesSecurityHistoricalDAO
    : public HistoricalDataAccessObject<SvcFeesSecurityHistoricalKey,
                                        std::vector<SvcFeesSecurityInfo*> >
{
public:
  static SvcFeesSecurityHistoricalDAO& instance();

  const std::vector<SvcFeesSecurityInfo*>&
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, SvcFeesSecurityHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b) &&
               objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    SvcFeesSecurityHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void setKeyDateRange(SvcFeesSecurityHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData*
  compress(const std::vector<SvcFeesSecurityInfo*>* vect) const override;

  virtual std::vector<SvcFeesSecurityInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<SvcFeesSecurityHistoricalDAO>;
  static DAOHelper<SvcFeesSecurityHistoricalDAO> _helper;
  SvcFeesSecurityHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<SvcFeesSecurityHistoricalKey, std::vector<SvcFeesSecurityInfo*> >(
          cacheSize, cacheType)
  {
  }
  std::vector<SvcFeesSecurityInfo*>* create(SvcFeesSecurityHistoricalKey key) override;
  void destroy(SvcFeesSecurityHistoricalKey key, std::vector<SvcFeesSecurityInfo*>* t) override;

private:
  static SvcFeesSecurityHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
