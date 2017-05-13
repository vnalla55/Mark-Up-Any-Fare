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
class SalesRestriction;
class DeleteList;

typedef HashKey<VendorCode, int> SalesRestrictionKey;

class SalesRestrictionDAO
    : public DataAccessObject<SalesRestrictionKey, std::vector<SalesRestriction*> >
{
public:
  static SalesRestrictionDAO& instance();
  const SalesRestriction*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, SalesRestrictionKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  SalesRestrictionKey createKey(SalesRestriction* info);

  void translateKey(const SalesRestrictionKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<SalesRestriction, SalesRestrictionDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<SalesRestriction*>* vect) const override;

  virtual std::vector<SalesRestriction*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<SalesRestrictionDAO>;
  static DAOHelper<SalesRestrictionDAO> _helper;
  SalesRestrictionDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<SalesRestrictionKey, std::vector<SalesRestriction*> >(
          cacheSize, cacheType, 2)
  {
  }
  void load() override;
  std::vector<SalesRestriction*>* create(SalesRestrictionKey key) override;
  void destroy(SalesRestrictionKey key, std::vector<SalesRestriction*>* t) override;

private:
  static SalesRestrictionDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: SalesRestrictionHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> SalesRestrictionHistoricalKey;

class SalesRestrictionHistoricalDAO
    : public HistoricalDataAccessObject<SalesRestrictionHistoricalKey,
                                        std::vector<SalesRestriction*> >
{
public:
  static SalesRestrictionHistoricalDAO& instance();
  const SalesRestriction*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, SalesRestrictionHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b) &&
               objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    SalesRestrictionHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  SalesRestrictionHistoricalKey createKey(SalesRestriction* info,
                                          const DateTime& startDate = DateTime::emptyDate(),
                                          const DateTime& endDate = DateTime::emptyDate());

  void translateKey(const SalesRestrictionHistoricalKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
    objectKey.setValue("STARTDATE", key._c);
    objectKey.setValue("ENDDATE", key._d);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserterWithDateRange<SalesRestriction, SalesRestrictionHistoricalDAO>(
               flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  void setKeyDateRange(SalesRestrictionHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<SalesRestriction*>* vect) const override;

  virtual std::vector<SalesRestriction*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<SalesRestrictionHistoricalDAO>;
  static DAOHelper<SalesRestrictionHistoricalDAO> _helper;
  SalesRestrictionHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<SalesRestrictionHistoricalKey, std::vector<SalesRestriction*> >(
          cacheSize, cacheType, 2)
  {
  }
  std::vector<SalesRestriction*>* create(SalesRestrictionHistoricalKey key) override;
  void destroy(SalesRestrictionHistoricalKey key, std::vector<SalesRestriction*>* t) override;
  void load() override;

private:
  static SalesRestrictionHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
