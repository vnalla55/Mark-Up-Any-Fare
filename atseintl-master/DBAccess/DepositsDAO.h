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
class Deposits;
class DeleteList;

typedef HashKey<VendorCode, int> DepositsKey;

class DepositsDAO : public DataAccessObject<DepositsKey, std::vector<Deposits*> >
{
public:
  static DepositsDAO& instance();
  const Deposits*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, DepositsKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  DepositsKey createKey(const Deposits* info);

  void translateKey(const DepositsKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<Deposits, DepositsDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<DepositsDAO>;
  static DAOHelper<DepositsDAO> _helper;
  DepositsDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<DepositsKey, std::vector<Deposits*> >(cacheSize, cacheType, 2)
  {
  }
  std::vector<Deposits*>* create(DepositsKey key) override;
  void destroy(DepositsKey key, std::vector<Deposits*>* t) override;
  void load() override;

private:
  static DepositsDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: DepositsHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> DepositsHistoricalKey;

class DepositsHistoricalDAO
    : public HistoricalDataAccessObject<DepositsHistoricalKey, std::vector<Deposits*> >
{
public:
  static DepositsHistoricalDAO& instance();
  const Deposits*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, DepositsHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._c);
    objectKey.getValue("ENDDATE", key._d);
    return key.initialized = objectKey.getValue("VENDOR", key._a)
                             && objectKey.getValue("ITEMNO", key._b);
  }

  bool translateKey(const ObjectKey& objectKey,
                    DepositsHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void setKeyDateRange(DepositsHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<DepositsHistoricalDAO>;
  static DAOHelper<DepositsHistoricalDAO> _helper;
  DepositsHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<DepositsHistoricalKey, std::vector<Deposits*> >(cacheSize,
                                                                                 cacheType)
  {
  }
  std::vector<Deposits*>* create(DepositsHistoricalKey key) override;
  void destroy(DepositsHistoricalKey key, std::vector<Deposits*>* t) override;

private:
  static DepositsHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class DepositsHistoricalDAO
} // namespace tse
