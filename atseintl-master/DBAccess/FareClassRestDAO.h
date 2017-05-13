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
class FareClassRestRule;
class DeleteList;

typedef HashKey<VendorCode, int> FareClassRestKey;

class FareClassRestDAO : public DataAccessObject<FareClassRestKey, std::vector<FareClassRestRule*> >
{
public:
  static FareClassRestDAO& instance();
  const std::vector<FareClassRestRule*>&
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareClassRestKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  FareClassRestKey createKey(FareClassRestRule* info);

  void translateKey(const FareClassRestKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<FareClassRestRule, FareClassRestDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareClassRestDAO>;
  static DAOHelper<FareClassRestDAO> _helper;
  FareClassRestDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FareClassRestKey, std::vector<FareClassRestRule*> >(cacheSize, cacheType)
  {
  }
  void load() override;
  std::vector<FareClassRestRule*>* create(FareClassRestKey key) override;
  void destroy(FareClassRestKey key, std::vector<FareClassRestRule*>* t) override;

private:
  struct isEffective;
  static FareClassRestDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: FareClassRestHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> FareClassRestHistoricalKey;

class FareClassRestHistoricalDAO
    : public HistoricalDataAccessObject<FareClassRestHistoricalKey,
                                        std::vector<FareClassRestRule*> >
{
public:
  static FareClassRestHistoricalDAO& instance();
  const std::vector<FareClassRestRule*>&
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareClassRestHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._c);
    objectKey.getValue("ENDDATE", key._d);
    return key.initialized = objectKey.getValue("VENDOR", key._a)
                             && objectKey.getValue("ITEMNO", key._b);
  }

  bool translateKey(const ObjectKey& objectKey,
                    FareClassRestHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void setKeyDateRange(FareClassRestHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareClassRestHistoricalDAO>;
  static DAOHelper<FareClassRestHistoricalDAO> _helper;
  FareClassRestHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FareClassRestHistoricalKey, std::vector<FareClassRestRule*> >(
          cacheSize, cacheType)
  {
  }

  std::vector<FareClassRestRule*>* create(FareClassRestHistoricalKey key) override;
  void destroy(FareClassRestHistoricalKey key, std::vector<FareClassRestRule*>* t) override;

private:
  struct isEffective;
  static FareClassRestHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
