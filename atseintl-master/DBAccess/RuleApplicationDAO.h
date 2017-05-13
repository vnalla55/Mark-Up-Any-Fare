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
class RuleApplication;
class DeleteList;

typedef HashKey<VendorCode, int> RuleApplicationKey;

class RuleApplicationDAO
    : public DataAccessObject<RuleApplicationKey, std::vector<RuleApplication*> >
{
public:
  static RuleApplicationDAO& instance();
  const RuleApplication*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, RuleApplicationKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  RuleApplicationKey createKey(RuleApplication* info);

  void translateKey(const RuleApplicationKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<RuleApplication, RuleApplicationDAO>(flatKey, objectKey).success();
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<RuleApplicationDAO>;
  static DAOHelper<RuleApplicationDAO> _helper;
  RuleApplicationDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<RuleApplicationKey, std::vector<RuleApplication*> >(cacheSize, cacheType)
  {
  }
  std::vector<RuleApplication*>* create(RuleApplicationKey key) override;
  void destroy(RuleApplicationKey key, std::vector<RuleApplication*>* t) override;

  virtual void load() override;

private:
  static RuleApplicationDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: RuleApplicationHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> RuleApplicationHistoricalKey;

class RuleApplicationHistoricalDAO
    : public HistoricalDataAccessObject<RuleApplicationHistoricalKey,
                                        std::vector<RuleApplication*> >
{
public:
  static RuleApplicationHistoricalDAO& instance();
  const RuleApplication*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, RuleApplicationHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b) &&
               objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    RuleApplicationHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void setKeyDateRange(RuleApplicationHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<RuleApplicationHistoricalDAO>;
  static DAOHelper<RuleApplicationHistoricalDAO> _helper;
  RuleApplicationHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<RuleApplicationHistoricalKey, std::vector<RuleApplication*> >(
          cacheSize, cacheType)
  {
  }
  std::vector<RuleApplication*>* create(RuleApplicationHistoricalKey key) override;
  void destroy(RuleApplicationHistoricalKey key, std::vector<RuleApplication*>* t) override;

private:
  static RuleApplicationHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
