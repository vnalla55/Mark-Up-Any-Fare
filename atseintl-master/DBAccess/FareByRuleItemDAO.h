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
class FareByRuleItemInfo;
class DeleteList;

typedef HashKey<VendorCode, int> FareByRuleItemKey;

class FareByRuleItemDAO
    : public DataAccessObject<FareByRuleItemKey, std::vector<FareByRuleItemInfo*> >
{
public:
  static FareByRuleItemDAO& instance();
  const FareByRuleItemInfo*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareByRuleItemKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  FareByRuleItemKey createKey(FareByRuleItemInfo* info);

  void translateKey(const FareByRuleItemKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<FareByRuleItemInfo, FareByRuleItemDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData*
  compress(const std::vector<FareByRuleItemInfo*>* vect) const override;

  virtual std::vector<FareByRuleItemInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareByRuleItemDAO>;
  static DAOHelper<FareByRuleItemDAO> _helper;
  FareByRuleItemDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FareByRuleItemKey, std::vector<FareByRuleItemInfo*> >(
          cacheSize, cacheType, 4)
  {
  }
  void load() override;
  std::vector<FareByRuleItemInfo*>* create(FareByRuleItemKey key) override;
  void destroy(FareByRuleItemKey key, std::vector<FareByRuleItemInfo*>* t) override;

private:
  static FareByRuleItemDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class FareByRuleItemDAO

// --------------------------------------------------
// Historical DAO: FareByRuleItemHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> FareByRuleItemHistoricalKey;

class FareByRuleItemHistoricalDAO
    : public HistoricalDataAccessObject<FareByRuleItemHistoricalKey,
                                        std::vector<FareByRuleItemInfo*> >
{
public:
  static FareByRuleItemHistoricalDAO& instance();
  const FareByRuleItemInfo*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareByRuleItemHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._c);
    objectKey.getValue("ENDDATE", key._d);
    return key.initialized = objectKey.getValue("VENDOR", key._a)
                             && objectKey.getValue("ITEMNO", key._b);
  }

  bool translateKey(const ObjectKey& objectKey,
                    FareByRuleItemHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  FareByRuleItemHistoricalKey createKey(FareByRuleItemInfo* info,
                                        const DateTime& startDate = DateTime::emptyDate(),
                                        const DateTime& endDate = DateTime::emptyDate());

  void translateKey(const FareByRuleItemHistoricalKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
    objectKey.setValue("STARTDATE", key._c);
    objectKey.setValue("ENDDATE", key._d);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserterWithDateRange<FareByRuleItemInfo, FareByRuleItemHistoricalDAO>(
               flatKey, objectKey).success();
  }

  void setKeyDateRange(FareByRuleItemHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  virtual sfc::CompressedData*
  compress(const std::vector<FareByRuleItemInfo*>* vect) const override;

  virtual std::vector<FareByRuleItemInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareByRuleItemHistoricalDAO>;
  static DAOHelper<FareByRuleItemHistoricalDAO> _helper;
  FareByRuleItemHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FareByRuleItemHistoricalKey, std::vector<FareByRuleItemInfo*> >(
          cacheSize, cacheType, 4)
  {
  }
  std::vector<FareByRuleItemInfo*>* create(FareByRuleItemHistoricalKey key) override;
  void destroy(FareByRuleItemHistoricalKey key, std::vector<FareByRuleItemInfo*>* t) override;
  void load() override;

private:
  static FareByRuleItemHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class FareByRuleItemHistoricalDAO
} // namespace tse
