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
class TravelRestriction;
class DeleteList;

typedef HashKey<VendorCode, int> TravelRestrictionKey;

class TravelRestrictionDAO
    : public DataAccessObject<TravelRestrictionKey, std::vector<TravelRestriction*> >
{
public:
  static TravelRestrictionDAO& instance();
  const TravelRestriction*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, TravelRestrictionKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  TravelRestrictionKey createKey(TravelRestriction* info);

  void translateKey(const TravelRestrictionKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<TravelRestriction, TravelRestrictionDAO>(flatKey, objectKey)
        .success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<TravelRestriction*>* vect) const override;

  virtual std::vector<TravelRestriction*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<TravelRestrictionDAO>;
  static DAOHelper<TravelRestrictionDAO> _helper;
  TravelRestrictionDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<TravelRestrictionKey, std::vector<TravelRestriction*> >(cacheSize, cacheType)
  {
  }
  void load() override;
  std::vector<TravelRestriction*>* create(TravelRestrictionKey key) override;
  void destroy(TravelRestrictionKey key, std::vector<TravelRestriction*>* t) override;

private:
  static TravelRestrictionDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: TravelRestrictionHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> TravelRestrictionHistoricalKey;

class TravelRestrictionHistoricalDAO
    : public HistoricalDataAccessObject<TravelRestrictionHistoricalKey,
                                        std::vector<TravelRestriction*> >
{
public:
  static TravelRestrictionHistoricalDAO& instance();
  const TravelRestriction*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, TravelRestrictionHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b) &&
               objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    TravelRestrictionHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void
  setKeyDateRange(TravelRestrictionHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  TravelRestrictionHistoricalKey createKey(const TravelRestriction* info,
                                           const DateTime& startDate = DateTime::emptyDate(),
                                           const DateTime& endDate = DateTime::emptyDate());

  void translateKey(const TravelRestrictionHistoricalKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
    objectKey.setValue("STARTDATE", key._c);
    objectKey.setValue("ENDDATE", key._d);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserterWithDateRange<TravelRestriction, TravelRestrictionHistoricalDAO>(
               flatKey, objectKey).success();
  }

  virtual sfc::CompressedData* compress(const std::vector<TravelRestriction*>* vect) const override;

  virtual std::vector<TravelRestriction*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<TravelRestrictionHistoricalDAO>;
  static DAOHelper<TravelRestrictionHistoricalDAO> _helper;
  TravelRestrictionHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<TravelRestrictionHistoricalKey, std::vector<TravelRestriction*> >(
          cacheSize, cacheType)
  {
  }
  std::vector<TravelRestriction*>* create(TravelRestrictionHistoricalKey key) override;
  void destroy(TravelRestrictionHistoricalKey key, std::vector<TravelRestriction*>* t) override;
  void load() override;

private:
  static TravelRestrictionHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
