//------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//------------------------------------------------------------------------------
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
class Brand;
class DeleteList;

typedef HashKey<Indicator, UserApplCode, CarrierCode> BrandKey;

class BrandDAO : public DataAccessObject<BrandKey, std::vector<Brand*>, false>
{
public:
  static BrandDAO& instance();

  const std::vector<Brand*>& get(DeleteList& del,
                                 const Indicator& userApplType,
                                 const UserApplCode& userAppl,
                                 const CarrierCode& carrier,
                                 const DateTime& travelDate,
                                 const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, BrandKey& key) const override
  {
    return key.initialized = objectKey.getValue("USERAPPLTYPE", key._a) &&
                             objectKey.getValue("USERAPPL", key._b) &&
                             objectKey.getValue("CARRIER", key._c);
  }

  BrandKey createKey(Brand* info);

  void translateKey(const BrandKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("USERAPPLTYPE", key._a);
    objectKey.setValue("USERAPPL", key._b);
    objectKey.setValue("CARRIER", key._c);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<Brand, BrandDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<BrandDAO>;

  static DAOHelper<BrandDAO> _helper;

  BrandDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<BrandKey, std::vector<Brand*>, false>(cacheSize, cacheType)
  {
  }

  std::vector<Brand*>* create(BrandKey key) override;
  void destroy(BrandKey key, std::vector<Brand*>* recs) override;

  virtual void load() override;

private:
  static BrandDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: BrandHistoricalDAO
// --------------------------------------------------
class BrandHistoricalDAO : public HistoricalDataAccessObject<BrandKey, std::vector<Brand*>, false>
{
public:
  static BrandHistoricalDAO& instance();

  const std::vector<Brand*>& get(DeleteList& del,
                                 const Indicator& userApplType,
                                 const UserApplCode& userAppl,
                                 const CarrierCode& carrier,
                                 const DateTime& travelDate,
                                 const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, BrandKey& key) const override
  {
    return key.initialized = objectKey.getValue("USERAPPLTYPE", key._a) &&
                             objectKey.getValue("USERAPPL", key._b) &&
                             objectKey.getValue("CARRIER", key._c);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<BrandHistoricalDAO>;

  static DAOHelper<BrandHistoricalDAO> _helper;

  BrandHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<BrandKey, std::vector<Brand*>, false>(cacheSize, cacheType)
  {
  }

  std::vector<Brand*>* create(BrandKey key) override;
  void destroy(BrandKey key, std::vector<Brand*>* recs) override;

  virtual void load() override;

private:
  struct groupByKey;
  static BrandHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse

