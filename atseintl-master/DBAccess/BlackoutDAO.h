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
class BlackoutInfo;
class DeleteList;

typedef HashKey<VendorCode, int> BlackoutKey;

class BlackoutDAO : public DataAccessObject<BlackoutKey, std::vector<BlackoutInfo*> >
{
public:
  static BlackoutDAO& instance();

  const BlackoutInfo*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, BlackoutKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  BlackoutKey createKey(BlackoutInfo* info);

  void translateKey(const BlackoutKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<BlackoutInfo, BlackoutDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<BlackoutInfo*>* vect) const override;

  virtual std::vector<BlackoutInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<BlackoutDAO>;
  static DAOHelper<BlackoutDAO> _helper;
  BlackoutDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<BlackoutKey, std::vector<BlackoutInfo*> >(cacheSize, cacheType)
  {
  }
  void load() override;
  std::vector<BlackoutInfo*>* create(BlackoutKey key) override;
  void destroy(BlackoutKey key, std::vector<BlackoutInfo*>* t) override;

private:
  static BlackoutDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: BlackoutHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> BlackoutHistoricalKey;

class BlackoutHistoricalDAO
    : public HistoricalDataAccessObject<BlackoutHistoricalKey, std::vector<BlackoutInfo*> >
{
public:
  static BlackoutHistoricalDAO& instance();

  const BlackoutInfo*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, BlackoutHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._c);
    objectKey.getValue("ENDDATE", key._d);
    return key.initialized = objectKey.getValue("VENDOR", key._a)
                             && objectKey.getValue("ITEMNO", key._b);
  }

  bool translateKey(const ObjectKey& objectKey,
                    BlackoutHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void setKeyDateRange(BlackoutHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<BlackoutInfo*>* vect) const override;

  virtual std::vector<BlackoutInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<BlackoutHistoricalDAO>;
  static DAOHelper<BlackoutHistoricalDAO> _helper;
  BlackoutHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<BlackoutHistoricalKey, std::vector<BlackoutInfo*> >(cacheSize,
                                                                                     cacheType)
  {
  }
  std::vector<BlackoutInfo*>* create(BlackoutHistoricalKey key) override;
  void destroy(BlackoutHistoricalKey key, std::vector<BlackoutInfo*>* t) override;

private:
  static BlackoutHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse

