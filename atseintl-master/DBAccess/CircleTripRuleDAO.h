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
#include "DBAccess/HashKey.h"
#include "DBAccess/HistoricalDataAccessObject.h"

#include <log4cxx/helpers/objectptr.h>

#include <vector>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class CircleTripRuleItem;
class DeleteList;

typedef HashKey<VendorCode, int> CircleTripRuleKey;

class CircleTripRuleDAO
    : public DataAccessObject<CircleTripRuleKey, std::vector<CircleTripRuleItem*> >
{
public:
  static CircleTripRuleDAO& instance();
  CircleTripRuleItem*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, CircleTripRuleKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  CircleTripRuleKey createKey(CircleTripRuleItem* info);

  void translateKey(const CircleTripRuleKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<CircleTripRuleItem, CircleTripRuleDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<CircleTripRuleDAO>;
  static DAOHelper<CircleTripRuleDAO> _helper;
  CircleTripRuleDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<CircleTripRuleKey, std::vector<CircleTripRuleItem*> >(cacheSize, cacheType)
  {
  }
  void load() override;
  std::vector<CircleTripRuleItem*>* create(CircleTripRuleKey key) override;
  void destroy(CircleTripRuleKey key, std::vector<CircleTripRuleItem*>* t) override;

private:
  static CircleTripRuleDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: CircleTripRuleHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> CircleTripRuleHistoricalKey;

class CircleTripRuleHistoricalDAO
    : public HistoricalDataAccessObject<CircleTripRuleHistoricalKey,
                                        std::vector<CircleTripRuleItem*> >
{
public:
  static CircleTripRuleHistoricalDAO& instance();
  CircleTripRuleItem*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, CircleTripRuleHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._c);
    objectKey.getValue("ENDDATE", key._d);
    return key.initialized = objectKey.getValue("VENDOR", key._a)
                             && objectKey.getValue("ITEMNO", key._b);
  }

  bool translateKey(const ObjectKey& objectKey,
                    CircleTripRuleHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void setKeyDateRange(CircleTripRuleHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<CircleTripRuleHistoricalDAO>;
  static DAOHelper<CircleTripRuleHistoricalDAO> _helper;
  CircleTripRuleHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<CircleTripRuleHistoricalKey, std::vector<CircleTripRuleItem*> >(
          cacheSize, cacheType)
  {
  }
  std::vector<CircleTripRuleItem*>* create(CircleTripRuleHistoricalKey key) override;
  void destroy(CircleTripRuleHistoricalKey key, std::vector<CircleTripRuleItem*>* t) override;

private:
  static CircleTripRuleHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
