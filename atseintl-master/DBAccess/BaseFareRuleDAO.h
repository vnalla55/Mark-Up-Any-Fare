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
class BaseFareRule;
class DeleteList;

typedef HashKey<VendorCode, int> BaseFareRuleKey;

class BaseFareRuleDAO : public DataAccessObject<BaseFareRuleKey, std::vector<const BaseFareRule*> >
{
public:
  static BaseFareRuleDAO& instance();
  const std::vector<const BaseFareRule*>& get(DeleteList& del,
                                              const VendorCode& vendor,
                                              int itemNo,
                                              const DateTime& date,
                                              const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, BaseFareRuleKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  BaseFareRuleKey createKey(const BaseFareRule* info);

  void translateKey(const BaseFareRuleKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<BaseFareRule, BaseFareRuleDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData*
  compress(const std::vector<const BaseFareRule*>* vect) const override;

  virtual std::vector<const BaseFareRule*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  struct isEffective;
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<BaseFareRuleDAO>;
  static DAOHelper<BaseFareRuleDAO> _helper;
  BaseFareRuleDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<BaseFareRuleKey, std::vector<const BaseFareRule*> >(cacheSize, cacheType, 3)
  {
  }
  void load() override;
  std::vector<const BaseFareRule*>* create(BaseFareRuleKey key) override;
  void destroy(BaseFareRuleKey key, std::vector<const BaseFareRule*>* t) override;

private:
  static BaseFareRuleDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: BaseFareRuleHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> BaseFareRuleHistoricalKey;

class BaseFareRuleHistoricalDAO
    : public HistoricalDataAccessObject<BaseFareRuleHistoricalKey,
                                        std::vector<const BaseFareRule*> >
{
public:
  static BaseFareRuleHistoricalDAO& instance();
  const std::vector<const BaseFareRule*>& get(DeleteList& del,
                                              const VendorCode& vendor,
                                              int itemNo,
                                              const DateTime& date,
                                              const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, BaseFareRuleHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._c);
    objectKey.getValue("ENDDATE", key._d);
    return key.initialized = objectKey.getValue("VENDOR", key._a)
                             && objectKey.getValue("ITEMNO", key._b);
  }

  bool translateKey(const ObjectKey& objectKey,
                    BaseFareRuleHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void setKeyDateRange(BaseFareRuleHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData*
  compress(const std::vector<const BaseFareRule*>* vect) const override;

  virtual std::vector<const BaseFareRule*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<BaseFareRuleHistoricalDAO>;
  static DAOHelper<BaseFareRuleHistoricalDAO> _helper;
  BaseFareRuleHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<BaseFareRuleHistoricalKey, std::vector<const BaseFareRule*> >(
          cacheSize, cacheType, 3)
  {
  }
  std::vector<const BaseFareRule*>* create(BaseFareRuleHistoricalKey key) override;
  void destroy(BaseFareRuleHistoricalKey key, std::vector<const BaseFareRule*>* t) override;

private:
  static BaseFareRuleHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse

