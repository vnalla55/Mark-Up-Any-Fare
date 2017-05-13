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
#include "DBAccess/HistoricalDataAccessObject.h"

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class OpenJawRule;
class DeleteList;

typedef HashKey<VendorCode, int> OpenJawRuleKey;

class OpenJawRuleDAO : public DataAccessObject<OpenJawRuleKey, std::vector<OpenJawRule*> >
{
public:
  static OpenJawRuleDAO& instance();
  const OpenJawRule*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, OpenJawRuleKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  OpenJawRuleKey createKey(OpenJawRule* info);

  void translateKey(const OpenJawRuleKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<OpenJawRule, OpenJawRuleDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<OpenJawRuleDAO>;
  static DAOHelper<OpenJawRuleDAO> _helper;
  OpenJawRuleDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<OpenJawRuleKey, std::vector<OpenJawRule*> >(cacheSize, cacheType)
  {
  }
  void load() override;
  std::vector<OpenJawRule*>* create(OpenJawRuleKey key) override;
  void destroy(OpenJawRuleKey key, std::vector<OpenJawRule*>* t) override;

private:
  static OpenJawRuleDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: OpenJawRuleHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> OpenJawRuleHistoricalKey;

class OpenJawRuleHistoricalDAO
    : public HistoricalDataAccessObject<OpenJawRuleHistoricalKey, std::vector<OpenJawRule*> >
{
public:
  static OpenJawRuleHistoricalDAO& instance();
  const OpenJawRule*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, OpenJawRuleHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b) &&
               objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    OpenJawRuleHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void setKeyDateRange(OpenJawRuleHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<OpenJawRuleHistoricalDAO>;
  static DAOHelper<OpenJawRuleHistoricalDAO> _helper;
  OpenJawRuleHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<OpenJawRuleHistoricalKey, std::vector<OpenJawRule*> >(cacheSize,
                                                                                       cacheType)
  {
  }
  std::vector<OpenJawRule*>* create(OpenJawRuleHistoricalKey key) override;
  void destroy(OpenJawRuleHistoricalKey key, std::vector<OpenJawRule*>* t) override;

private:
  static OpenJawRuleHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
