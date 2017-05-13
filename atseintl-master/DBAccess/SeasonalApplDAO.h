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
class SeasonalAppl;
class DeleteList;

typedef HashKey<VendorCode, int> SeasonalApplKey;

class SeasonalApplDAO : public DataAccessObject<SeasonalApplKey, std::vector<SeasonalAppl*> >
{
public:
  static SeasonalApplDAO& instance();
  const SeasonalAppl*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, SeasonalApplKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  SeasonalApplKey createKey(SeasonalAppl* info);

  void translateKey(const SeasonalApplKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<SeasonalAppl, SeasonalApplDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<SeasonalAppl*>* vect) const override;

  virtual std::vector<SeasonalAppl*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<SeasonalApplDAO>;
  static DAOHelper<SeasonalApplDAO> _helper;
  SeasonalApplDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<SeasonalApplKey, std::vector<SeasonalAppl*> >(cacheSize, cacheType)
  {
  }
  void load() override;
  std::vector<SeasonalAppl*>* create(SeasonalApplKey key) override;
  void destroy(SeasonalApplKey key, std::vector<SeasonalAppl*>* t) override;

private:
  static SeasonalApplDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: SeasonalApplHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> SeasonalApplHistoricalKey;

class SeasonalApplHistoricalDAO
    : public HistoricalDataAccessObject<SeasonalApplHistoricalKey, std::vector<SeasonalAppl*> >
{
public:
  static SeasonalApplHistoricalDAO& instance();
  const SeasonalAppl*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, SeasonalApplHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b) &&
               objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    SeasonalApplHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  SeasonalApplHistoricalKey createKey(SeasonalAppl* info,
                                      const DateTime& startDate = DateTime::emptyDate(),
                                      const DateTime& endDate = DateTime::emptyDate());

  void translateKey(const SeasonalApplHistoricalKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
    objectKey.setValue("STARTDATE", key._c);
    objectKey.setValue("ENDDATE", key._d);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserterWithDateRange<SeasonalAppl, SeasonalApplHistoricalDAO>(
               flatKey, objectKey).success();
  }

  void setKeyDateRange(SeasonalApplHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  virtual sfc::CompressedData* compress(const std::vector<SeasonalAppl*>* vect) const override;

  virtual std::vector<SeasonalAppl*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<SeasonalApplHistoricalDAO>;
  static DAOHelper<SeasonalApplHistoricalDAO> _helper;
  SeasonalApplHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<SeasonalApplHistoricalKey, std::vector<SeasonalAppl*> >(cacheSize,
                                                                                         cacheType)
  {
  }
  std::vector<SeasonalAppl*>* create(SeasonalApplHistoricalKey key) override;
  void destroy(SeasonalApplHistoricalKey key, std::vector<SeasonalAppl*>* t) override;
  void load() override;

private:
  static SeasonalApplHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
