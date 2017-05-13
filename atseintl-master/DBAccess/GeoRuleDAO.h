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
class GeoRuleItem;
class DeleteList;

typedef HashKey<VendorCode, int> GeoRuleKey;

class GeoRuleDAO : public DataAccessObject<GeoRuleKey, std::vector<GeoRuleItem*> >
{
public:
  static GeoRuleDAO& instance();
  const std::vector<GeoRuleItem*>&
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, GeoRuleKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  GeoRuleKey createKey(GeoRuleItem* info);

  void translateKey(const GeoRuleKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<GeoRuleItem, GeoRuleDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<GeoRuleItem*>* vect) const override;

  virtual std::vector<GeoRuleItem*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<GeoRuleDAO>;
  static DAOHelper<GeoRuleDAO> _helper;
  GeoRuleDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<GeoRuleKey, std::vector<GeoRuleItem*> >(cacheSize, cacheType)
  {
  }
  void load() override;
  std::vector<GeoRuleItem*>* create(GeoRuleKey key) override;
  void destroy(GeoRuleKey key, std::vector<GeoRuleItem*>* t) override;

private:
  static GeoRuleDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: GeoRuleHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> GeoRuleHistoricalKey;

class GeoRuleHistoricalDAO
    : public HistoricalDataAccessObject<GeoRuleHistoricalKey, std::vector<GeoRuleItem*> >
{
public:
  static GeoRuleHistoricalDAO& instance();
  const std::vector<GeoRuleItem*>&
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, GeoRuleHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b) &&
               objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    GeoRuleHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void setKeyDateRange(GeoRuleHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<GeoRuleItem*>* vect) const override;

  virtual std::vector<GeoRuleItem*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<GeoRuleHistoricalDAO>;
  static DAOHelper<GeoRuleHistoricalDAO> _helper;
  GeoRuleHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<GeoRuleHistoricalKey, std::vector<GeoRuleItem*> >(cacheSize,
                                                                                   cacheType)
  {
  }
  std::vector<GeoRuleItem*>* create(GeoRuleHistoricalKey key) override;
  void destroy(GeoRuleHistoricalKey key, std::vector<GeoRuleItem*>* t) override;

private:
  static GeoRuleHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
