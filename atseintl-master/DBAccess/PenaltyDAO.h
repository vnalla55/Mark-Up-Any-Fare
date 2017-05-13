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
class PenaltyInfo;
class DeleteList;

typedef HashKey<VendorCode, int> PenaltyKey;

class PenaltyDAO : public DataAccessObject<PenaltyKey, std::vector<PenaltyInfo*> >
{
public:
  static PenaltyDAO& instance();
  const PenaltyInfo*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, PenaltyKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  PenaltyKey createKey(PenaltyInfo* info);

  void translateKey(const PenaltyKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<PenaltyInfo, PenaltyDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<PenaltyInfo*>* vect) const override;

  virtual std::vector<PenaltyInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<PenaltyDAO>;
  static DAOHelper<PenaltyDAO> _helper;
  PenaltyDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<PenaltyKey, std::vector<PenaltyInfo*> >(cacheSize, cacheType)
  {
  }
  void load() override;
  std::vector<PenaltyInfo*>* create(PenaltyKey key) override;
  void destroy(PenaltyKey key, std::vector<PenaltyInfo*>* t) override;

private:
  static PenaltyDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: PenaltyHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> PenaltyHistoricalKey;

class PenaltyHistoricalDAO
    : public HistoricalDataAccessObject<PenaltyHistoricalKey, std::vector<PenaltyInfo*> >
{
public:
  static PenaltyHistoricalDAO& instance();
  const PenaltyInfo*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, PenaltyHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b) &&
               objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    PenaltyHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  PenaltyHistoricalKey createKey(PenaltyInfo* info,
                                 const DateTime& startDate = DateTime::emptyDate(),
                                 const DateTime& endDate = DateTime::emptyDate());

  void translateKey(const PenaltyHistoricalKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
    objectKey.setValue("STARTDATE", key._c);
    objectKey.setValue("ENDDATE", key._d);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserterWithDateRange<PenaltyInfo, PenaltyHistoricalDAO>(flatKey, objectKey)
        .success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  void setKeyDateRange(PenaltyHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<PenaltyInfo*>* vect) const override;

  virtual std::vector<PenaltyInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<PenaltyHistoricalDAO>;
  static DAOHelper<PenaltyHistoricalDAO> _helper;
  PenaltyHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<PenaltyHistoricalKey, std::vector<PenaltyInfo*> >(cacheSize,
                                                                                   cacheType)
  {
  }
  std::vector<PenaltyInfo*>* create(PenaltyHistoricalKey key) override;
  void destroy(PenaltyHistoricalKey key, std::vector<PenaltyInfo*>* t) override;
  void load() override;

private:
  static PenaltyHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
