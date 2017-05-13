//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
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
class OptionalServicesActivationInfo;

typedef HashKey<Indicator, UserApplCode, std::string> OptServActivationKey;
typedef HashKey<Indicator, UserApplCode, std::string, DateTime, DateTime>
OptServActivationHistoricalKey;

class OptionalServicesActivationDAO
    : public DataAccessObject<OptServActivationKey, std::vector<OptionalServicesActivationInfo*> >
{
public:
  friend class DAOHelper<OptionalServicesActivationDAO>;

  static OptionalServicesActivationDAO& instance();

  const std::vector<OptionalServicesActivationInfo*>& get(DeleteList& del,
                                                          Indicator crs,
                                                          const UserApplCode& userCode,
                                                          const std::string& application,
                                                          const DateTime& ticketDate);

  OptServActivationKey createKey(OptionalServicesActivationInfo* info);

  bool translateKey(const ObjectKey& objectKey, OptServActivationKey& key) const override
  {
    return key.initialized = objectKey.getValue("USERAPPLTYPE", key._a) &&
                             objectKey.getValue("USERAPPL", key._b) &&
                             objectKey.getValue("APPLICATION", key._c);
  }

  void translateKey(const OptServActivationKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("USERAPPLTYPE", key._a);
    objectKey.setValue("USERAPPL", key._b);
    objectKey.setValue("APPLICATION", key._c);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<OptionalServicesActivationInfo, OptionalServicesActivationDAO>(
               flatKey, objectKey).success();
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

private:
  OptionalServicesActivationDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<OptServActivationKey, std::vector<OptionalServicesActivationInfo*> >(
          cacheSize, cacheType, 2)
  {
  }

  std::vector<OptionalServicesActivationInfo*>* create(OptServActivationKey key) override;
  void
  destroy(OptServActivationKey key, std::vector<OptionalServicesActivationInfo*>* recs) override;
  void load() override;

  static std::string _name;
  static std::string _cacheClass;
  static DAOHelper<OptionalServicesActivationDAO> _helper;
  static OptionalServicesActivationDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

class OptionalServicesActivationHistoricalDAO
    : public HistoricalDataAccessObject<OptServActivationHistoricalKey,
                                        std::vector<OptionalServicesActivationInfo*> >
{
public:
  friend class DAOHelper<OptionalServicesActivationHistoricalDAO>;

  static OptionalServicesActivationHistoricalDAO& instance();

  const std::vector<OptionalServicesActivationInfo*>& get(DeleteList& del,
                                                          Indicator crs,
                                                          const UserApplCode& userCode,
                                                          const std::string& application,
                                                          const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, OptServActivationHistoricalKey& key) const override
  {
    return key.initialized = objectKey.getValue("USERAPPLTYPE", key._a) &&
                             objectKey.getValue("USERAPPL", key._b) &&
                             objectKey.getValue("APPLICATION", key._c) &&
                             objectKey.getValue("STARTDATE", key._d) &&
                             objectKey.getValue("ENDDATE", key._e);
  }

  bool translateKey(const ObjectKey& objectKey,
                    OptServActivationHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);

    return key.initialized = objectKey.getValue("USERAPPLTYPE", key._a) &&
                             objectKey.getValue("USERAPPL", key._b) &&
                             objectKey.getValue("APPLICATION", key._c);
  }

  void
  setKeyDateRange(OptServActivationHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

private:
  OptionalServicesActivationHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<OptServActivationHistoricalKey,
                                 std::vector<OptionalServicesActivationInfo*> >(
          cacheSize, cacheType, 2)
  {
  }
  std::vector<OptionalServicesActivationInfo*>* create(OptServActivationHistoricalKey key) override;
  void destroy(OptServActivationHistoricalKey key,
               std::vector<OptionalServicesActivationInfo*>* t) override;

  static std::string _name;
  static std::string _cacheClass;
  static DAOHelper<OptionalServicesActivationHistoricalDAO> _helper;
  static OptionalServicesActivationHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // tse

