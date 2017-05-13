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
class TariffRuleRest;
class DeleteList;

typedef HashKey<VendorCode, int> TariffRuleRestKey;

class TariffRuleRestDAO : public DataAccessObject<TariffRuleRestKey, std::vector<TariffRuleRest*> >
{
public:
  static TariffRuleRestDAO& instance();
  const std::vector<TariffRuleRest*>&
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, TariffRuleRestKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  TariffRuleRestKey createKey(TariffRuleRest* info);

  void translateKey(const TariffRuleRestKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<TariffRuleRest, TariffRuleRestDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<TariffRuleRest*>* vect) const override;

  virtual std::vector<TariffRuleRest*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<TariffRuleRestDAO>;
  static DAOHelper<TariffRuleRestDAO> _helper;
  TariffRuleRestDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<TariffRuleRestKey, std::vector<TariffRuleRest*> >(cacheSize, cacheType)
  {
  }
  void load() override;
  std::vector<TariffRuleRest*>* create(TariffRuleRestKey key) override;
  void destroy(TariffRuleRestKey key, std::vector<TariffRuleRest*>* t) override;

private:
  static TariffRuleRestDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: TariffRuleRestHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> TariffRuleRestHistoricalKey;

class TariffRuleRestHistoricalDAO
    : public HistoricalDataAccessObject<TariffRuleRestHistoricalKey, std::vector<TariffRuleRest*> >
{
public:
  static TariffRuleRestHistoricalDAO& instance();
  const std::vector<TariffRuleRest*>&
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, TariffRuleRestHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b) &&
               objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    TariffRuleRestHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void setKeyDateRange(TariffRuleRestHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<TariffRuleRest*>* vect) const override;

  virtual std::vector<TariffRuleRest*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<TariffRuleRestHistoricalDAO>;
  static DAOHelper<TariffRuleRestHistoricalDAO> _helper;
  TariffRuleRestHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<TariffRuleRestHistoricalKey, std::vector<TariffRuleRest*> >(
          cacheSize, cacheType)
  {
  }
  std::vector<TariffRuleRest*>* create(TariffRuleRestHistoricalKey key) override;
  void destroy(TariffRuleRestHistoricalKey key, std::vector<TariffRuleRest*>* t) override;

private:
  static TariffRuleRestHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
