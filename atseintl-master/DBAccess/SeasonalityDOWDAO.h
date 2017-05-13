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
class SeasonalityDOW;
class DeleteList;

typedef HashKey<VendorCode, int> SeasonalityDOWKey;

class SeasonalityDOWDAO : public DataAccessObject<SeasonalityDOWKey, std::vector<SeasonalityDOW*> >
{
public:
  static SeasonalityDOWDAO& instance();
  const SeasonalityDOW*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& date);

  bool translateKey(const ObjectKey& objectKey, SeasonalityDOWKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  SeasonalityDOWKey createKey(SeasonalityDOW* info);

  void translateKey(const SeasonalityDOWKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<SeasonalityDOW, SeasonalityDOWDAO>(flatKey, objectKey).success();
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<SeasonalityDOWDAO>;
  static DAOHelper<SeasonalityDOWDAO> _helper;
  SeasonalityDOWDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<SeasonalityDOWKey, std::vector<SeasonalityDOW*> >(cacheSize, cacheType)
  {
  }
  virtual void load() override;
  std::vector<SeasonalityDOW*>* create(SeasonalityDOWKey key) override;
  void destroy(SeasonalityDOWKey key, std::vector<SeasonalityDOW*>* t) override;

private:
  static SeasonalityDOWDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

class SeasonalityDOWHistoricalDAO
    : public HistoricalDataAccessObject<SeasonalityDOWKey, std::vector<SeasonalityDOW*> >
{
public:
  static SeasonalityDOWHistoricalDAO& instance();
  const SeasonalityDOW*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, SeasonalityDOWKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<SeasonalityDOWHistoricalDAO>;
  static DAOHelper<SeasonalityDOWHistoricalDAO> _helper;
  SeasonalityDOWHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<SeasonalityDOWKey, std::vector<SeasonalityDOW*> >(cacheSize,
                                                                                   cacheType)
  {
  }
  std::vector<SeasonalityDOW*>* create(SeasonalityDOWKey key) override;
  void destroy(SeasonalityDOWKey key, std::vector<SeasonalityDOW*>* vc) override;

private:
  static SeasonalityDOWHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
}
