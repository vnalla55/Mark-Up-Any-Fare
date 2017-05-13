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
class HipMileageExceptInfo;
class DeleteList;

typedef HashKey<VendorCode, int> HipMileageExceptKey;

class HipMileageExceptDAO
    : public DataAccessObject<HipMileageExceptKey, std::vector<const HipMileageExceptInfo*> >
{
public:
  static HipMileageExceptDAO& instance();
  const HipMileageExceptInfo*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, HipMileageExceptKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  HipMileageExceptKey createKey(const HipMileageExceptInfo* info);

  void translateKey(const HipMileageExceptKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<HipMileageExceptInfo, HipMileageExceptDAO>(flatKey, objectKey)
        .success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<HipMileageExceptDAO>;
  static DAOHelper<HipMileageExceptDAO> _helper;
  HipMileageExceptDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<HipMileageExceptKey, std::vector<const HipMileageExceptInfo*> >(cacheSize,
                                                                                       cacheType)
  {
  }
  std::vector<const HipMileageExceptInfo*>* create(HipMileageExceptKey key) override;
  void destroy(HipMileageExceptKey key, std::vector<const HipMileageExceptInfo*>* t) override;
  void load() override;

private:
  static HipMileageExceptDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: HipMileageExceptHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> HipMileageExceptHistoricalKey;

class HipMileageExceptHistoricalDAO
    : public HistoricalDataAccessObject<HipMileageExceptHistoricalKey,
                                        std::vector<const HipMileageExceptInfo*> >
{
public:
  static HipMileageExceptHistoricalDAO& instance();
  const HipMileageExceptInfo*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, HipMileageExceptHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b) &&
               objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    HipMileageExceptHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void setKeyDateRange(HipMileageExceptHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<HipMileageExceptHistoricalDAO>;
  static DAOHelper<HipMileageExceptHistoricalDAO> _helper;
  HipMileageExceptHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<HipMileageExceptHistoricalKey,
                                 std::vector<const HipMileageExceptInfo*> >(cacheSize, cacheType)
  {
  }
  std::vector<const HipMileageExceptInfo*>* create(HipMileageExceptHistoricalKey key) override;
  void
  destroy(HipMileageExceptHistoricalKey key, std::vector<const HipMileageExceptInfo*>* t) override;

private:
  static HipMileageExceptHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
