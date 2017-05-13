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
class SamePoint;
class DeleteList;

typedef HashKey<VendorCode, int> SamePointKey;

class SamePointDAO : public DataAccessObject<SamePointKey, std::vector<const SamePoint*> >
{
public:
  static SamePointDAO& instance();
  const std::vector<const SamePoint*>&
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, SamePointKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  SamePointKey createKey(const SamePoint* info);

  void translateKey(const SamePointKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<SamePoint, SamePointDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<SamePointDAO>;
  static DAOHelper<SamePointDAO> _helper;
  SamePointDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<SamePointKey, std::vector<const SamePoint*> >(cacheSize, cacheType, 2)
  {
  }
  void load() override;
  std::vector<const SamePoint*>* create(SamePointKey key) override;
  void destroy(SamePointKey key, std::vector<const SamePoint*>* t) override;

private:
  static SamePointDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: SamePointHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> SamePointHistoricalKey;

class SamePointHistoricalDAO
    : public HistoricalDataAccessObject<SamePointHistoricalKey, std::vector<const SamePoint*> >
{
public:
  static SamePointHistoricalDAO& instance();
  const std::vector<const SamePoint*>&
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, SamePointHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b) &&
               objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    SamePointHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void setKeyDateRange(SamePointHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<SamePointHistoricalDAO>;
  static DAOHelper<SamePointHistoricalDAO> _helper;
  SamePointHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<SamePointHistoricalKey, std::vector<const SamePoint*> >(
          cacheSize, cacheType, 2)
  {
  }
  std::vector<const SamePoint*>* create(SamePointHistoricalKey key) override;
  void destroy(SamePointHistoricalKey key, std::vector<const SamePoint*>* t) override;

private:
  static SamePointHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
