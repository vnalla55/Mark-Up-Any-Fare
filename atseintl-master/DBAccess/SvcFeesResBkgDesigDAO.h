//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

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
class SvcFeesResBkgDesigInfo;
class DeleteList;

typedef HashKey<VendorCode, int> SvcFeesResBkgDesigKey;

class SvcFeesResBkgDesigDAO
    : public DataAccessObject<SvcFeesResBkgDesigKey, std::vector<SvcFeesResBkgDesigInfo*> >
{
public:
  static SvcFeesResBkgDesigDAO& instance();

  const std::vector<SvcFeesResBkgDesigInfo*>&
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, SvcFeesResBkgDesigKey& key) const override
  {
    return (key.initialized =
                (objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b)));
  }

  SvcFeesResBkgDesigKey createKey(SvcFeesResBkgDesigInfo* info);

  void translateKey(const SvcFeesResBkgDesigKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<SvcFeesResBkgDesigInfo, SvcFeesResBkgDesigDAO>(flatKey, objectKey)
        .success();
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<SvcFeesResBkgDesigDAO>;
  static DAOHelper<SvcFeesResBkgDesigDAO> _helper;
  SvcFeesResBkgDesigDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<SvcFeesResBkgDesigKey, std::vector<SvcFeesResBkgDesigInfo*> >(cacheSize,
                                                                                     cacheType)
  {
  }
  std::vector<SvcFeesResBkgDesigInfo*>* create(SvcFeesResBkgDesigKey key) override;
  void destroy(SvcFeesResBkgDesigKey key, std::vector<SvcFeesResBkgDesigInfo*>* recs) override;

  void load() override;

private:
  static SvcFeesResBkgDesigDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

typedef HashKey<VendorCode, int, DateTime, DateTime> SvcFeesResBkgDesigHistoricalKey;

class SvcFeesResBkgDesigHistoricalDAO
    : public HistoricalDataAccessObject<SvcFeesResBkgDesigHistoricalKey,
                                        std::vector<SvcFeesResBkgDesigInfo*> >
{
public:
  static SvcFeesResBkgDesigHistoricalDAO& instance();

  const std::vector<SvcFeesResBkgDesigInfo*>&
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, SvcFeesResBkgDesigHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b) &&
               objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    SvcFeesResBkgDesigHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void
  setKeyDateRange(SvcFeesResBkgDesigHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<SvcFeesResBkgDesigHistoricalDAO>;
  static DAOHelper<SvcFeesResBkgDesigHistoricalDAO> _helper;
  SvcFeesResBkgDesigHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<SvcFeesResBkgDesigHistoricalKey,
                                 std::vector<SvcFeesResBkgDesigInfo*> >(cacheSize, cacheType)
  {
  }
  std::vector<SvcFeesResBkgDesigInfo*>* create(SvcFeesResBkgDesigHistoricalKey key) override;
  void
  destroy(SvcFeesResBkgDesigHistoricalKey key, std::vector<SvcFeesResBkgDesigInfo*>* t) override;

private:
  static SvcFeesResBkgDesigHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
