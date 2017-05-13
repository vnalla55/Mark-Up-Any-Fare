//----------------------------------------------------------------------------
//  (C) 2009, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------

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
class FareTypeTable;
class DeleteList;

typedef HashKey<VendorCode, int> FareTypeTableKey;

class FareTypeTableDAO : public DataAccessObject<FareTypeTableKey, std::vector<FareTypeTable*> >
{
public:
  static FareTypeTableDAO& instance();
  std::vector<FareTypeTable*>&
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& date);

  bool translateKey(const ObjectKey& objectKey, FareTypeTableKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  FareTypeTableKey createKey(FareTypeTable* info);

  void translateKey(const FareTypeTableKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<FareTypeTable, FareTypeTableDAO>(flatKey, objectKey).success();
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareTypeTableDAO>;
  static DAOHelper<FareTypeTableDAO> _helper;
  FareTypeTableDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FareTypeTableKey, std::vector<FareTypeTable*> >(cacheSize, cacheType, 2)
  {
  }
  virtual void load() override;
  std::vector<FareTypeTable*>* create(FareTypeTableKey key) override;
  void destroy(FareTypeTableKey key, std::vector<FareTypeTable*>* t) override;

private:
  static FareTypeTableDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

class FareTypeTableHistoricalDAO
    : public HistoricalDataAccessObject<FareTypeTableKey, std::vector<FareTypeTable*> >
{
public:
  static FareTypeTableHistoricalDAO& instance();
  std::vector<FareTypeTable*>&
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareTypeTableKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareTypeTableHistoricalDAO>;
  static DAOHelper<FareTypeTableHistoricalDAO> _helper;
  FareTypeTableHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FareTypeTableKey, std::vector<FareTypeTable*> >(
          cacheSize, cacheType, 2)
  {
  }
  std::vector<FareTypeTable*>* create(FareTypeTableKey key) override;
  void destroy(FareTypeTableKey key, std::vector<FareTypeTable*>* vc) override;

private:
  static FareTypeTableHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
}
