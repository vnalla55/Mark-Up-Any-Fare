//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOUtils.h"
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
class SvcFeesAccCodeInfo;
class DeleteList;

typedef HashKey<VendorCode, int> SvcFeesAccountCodeKey;

class SvcFeesAccountCodeDAO
    : public DataAccessObject<SvcFeesAccountCodeKey, std::vector<SvcFeesAccCodeInfo*> >
{
public:
  static SvcFeesAccountCodeDAO& instance();

  const std::vector<SvcFeesAccCodeInfo*>&
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, SvcFeesAccountCodeKey& key) const override
  {
    return (key.initialized =
                (objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b)));
  }

  SvcFeesAccountCodeKey createKey(SvcFeesAccCodeInfo* info);

  void translateKey(const SvcFeesAccountCodeKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<SvcFeesAccCodeInfo, SvcFeesAccountCodeDAO>(flatKey, objectKey)
        .success();
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<SvcFeesAccountCodeDAO>;
  static DAOHelper<SvcFeesAccountCodeDAO> _helper;
  SvcFeesAccountCodeDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<SvcFeesAccountCodeKey, std::vector<SvcFeesAccCodeInfo*> >(
          cacheSize, cacheType, 2)
  {
  }
  std::vector<SvcFeesAccCodeInfo*>* create(SvcFeesAccountCodeKey key) override;
  void destroy(SvcFeesAccountCodeKey key, std::vector<SvcFeesAccCodeInfo*>* recs) override;

  void load() override;

private:
  static SvcFeesAccountCodeDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

typedef HashKey<VendorCode, int, DateTime, DateTime> SvcFeesAccountCodeHistoricalKey;

class SvcFeesAccountCodeHistoricalDAO
    : public HistoricalDataAccessObject<SvcFeesAccountCodeHistoricalKey,
                                        std::vector<SvcFeesAccCodeInfo*> >
{
public:
  static SvcFeesAccountCodeHistoricalDAO& instance();

  const std::vector<SvcFeesAccCodeInfo*>&
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, SvcFeesAccountCodeHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b) &&
               objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    SvcFeesAccountCodeHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void
  setKeyDateRange(SvcFeesAccountCodeHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<SvcFeesAccountCodeHistoricalDAO>;
  static DAOHelper<SvcFeesAccountCodeHistoricalDAO> _helper;
  SvcFeesAccountCodeHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<SvcFeesAccountCodeHistoricalKey,
                                 std::vector<SvcFeesAccCodeInfo*> >(cacheSize, cacheType, 2)
  {
  }
  std::vector<SvcFeesAccCodeInfo*>* create(SvcFeesAccountCodeHistoricalKey key) override;
  void destroy(SvcFeesAccountCodeHistoricalKey key, std::vector<SvcFeesAccCodeInfo*>* t) override;

private:
  static SvcFeesAccountCodeHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
