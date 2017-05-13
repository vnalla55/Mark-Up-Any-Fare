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
class DateOverrideRuleItem;
class DeleteList;

typedef HashKey<VendorCode, int> DateOverrideRuleKey;

class DateOverrideRuleDAO
    : public DataAccessObject<DateOverrideRuleKey, std::vector<DateOverrideRuleItem*> >
{
public:
  static DateOverrideRuleDAO& instance();
  const std::vector<DateOverrideRuleItem*>&
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, DateOverrideRuleKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  DateOverrideRuleKey createKey(DateOverrideRuleItem* info);

  void translateKey(const DateOverrideRuleKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<DateOverrideRuleItem, DateOverrideRuleDAO>(flatKey, objectKey)
        .success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData*
  compress(const std::vector<DateOverrideRuleItem*>* vect) const override;

  virtual std::vector<DateOverrideRuleItem*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<DateOverrideRuleDAO>;
  static DAOHelper<DateOverrideRuleDAO> _helper;
  DateOverrideRuleDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<DateOverrideRuleKey, std::vector<DateOverrideRuleItem*> >(cacheSize,
                                                                                 cacheType)
  {
  }
  void load() override;
  std::vector<DateOverrideRuleItem*>* create(DateOverrideRuleKey key) override;
  void destroy(DateOverrideRuleKey key, std::vector<DateOverrideRuleItem*>* t) override;

private:
  static DateOverrideRuleDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class DateOverrideRuleDAO

// --------------------------------------------------
// Historical DAO: DateOverrideRuleHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> DateOverrideRuleHistoricalKey;

class DateOverrideRuleHistoricalDAO
    : public HistoricalDataAccessObject<DateOverrideRuleHistoricalKey,
                                        std::vector<DateOverrideRuleItem*> >
{
public:
  static DateOverrideRuleHistoricalDAO& instance();
  const std::vector<DateOverrideRuleItem*>&
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, DateOverrideRuleHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._c);
    objectKey.getValue("ENDDATE", key._d);
    return key.initialized = objectKey.getValue("VENDOR", key._a)
                             && objectKey.getValue("ITEMNO", key._b);
  }

  bool translateKey(const ObjectKey& objectKey,
                    DateOverrideRuleHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void setKeyDateRange(DateOverrideRuleHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData*
  compress(const std::vector<DateOverrideRuleItem*>* vect) const override;

  virtual std::vector<DateOverrideRuleItem*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<DateOverrideRuleHistoricalDAO>;
  static DAOHelper<DateOverrideRuleHistoricalDAO> _helper;
  DateOverrideRuleHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<DateOverrideRuleHistoricalKey,
                                 std::vector<DateOverrideRuleItem*> >(cacheSize, cacheType)
  {
  }
  std::vector<DateOverrideRuleItem*>* create(DateOverrideRuleHistoricalKey key) override;
  void destroy(DateOverrideRuleHistoricalKey key, std::vector<DateOverrideRuleItem*>* t) override;

private:
  static DateOverrideRuleHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class DateOverrideRuleHistoricalDAO
} // namespace tse
