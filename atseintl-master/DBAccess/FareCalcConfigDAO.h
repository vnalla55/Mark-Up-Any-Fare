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
class FareCalcConfig;
class DeleteList;

typedef HashKey<Indicator, UserApplCode, PseudoCityCode> FareCalcConfigKey;

class FareCalcConfigDAO
    : public DataAccessObject<FareCalcConfigKey, std::vector<FareCalcConfig*>, false>
{
public:
  static FareCalcConfigDAO& instance();
  const std::vector<FareCalcConfig*>& get(DeleteList& del,
                                          Indicator userApplType,
                                          const UserApplCode& userAppl,
                                          const PseudoCityCode& pseudoCity,
                                          const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareCalcConfigKey& key) const override
  {
    return key.initialized = objectKey.getValue("USERAPPL", key._a) &&
                             objectKey.getValue("USERAPPLCODE", key._b) &&
                             objectKey.getValue("PSEUDOCITYCODE", key._c);
  }

  FareCalcConfigKey createKey(FareCalcConfig* info);

  void translateKey(const FareCalcConfigKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("USERAPPL", key._a);
    objectKey.setValue("USERAPPLCODE", key._b);
    objectKey.setValue("PSEUDOCITYCODE", key._c);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<FareCalcConfig, FareCalcConfigDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<FareCalcConfig*>* vect) const override;

  virtual std::vector<FareCalcConfig*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareCalcConfigDAO>;
  static DAOHelper<FareCalcConfigDAO> _helper;
  FareCalcConfigDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FareCalcConfigKey, std::vector<FareCalcConfig*>, false>(
          cacheSize, cacheType, 3)
  {
  }
  virtual void load() override;
  std::vector<FareCalcConfig*>* create(FareCalcConfigKey key) override;
  void destroy(FareCalcConfigKey key, std::vector<FareCalcConfig*>* t) override;

private:
  static FareCalcConfigDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: FareCalcConfigHistoricalDAO
// --------------------------------------------------
class FareCalcConfigHistoricalDAO
    : public HistoricalDataAccessObject<FareCalcConfigKey, std::vector<FareCalcConfig*>, false>
{
public:
  static FareCalcConfigHistoricalDAO& instance();
  const std::vector<FareCalcConfig*>& get(DeleteList& del,
                                          Indicator userApplType,
                                          const UserApplCode& userAppl,
                                          const PseudoCityCode& pseudoCity,
                                          const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareCalcConfigKey& key) const override
  {
    return key.initialized = objectKey.getValue("USERAPPL", key._a) &&
                             objectKey.getValue("USERAPPLCODE", key._b) &&
                             objectKey.getValue("PSEUDOCITYCODE", key._c);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  FareCalcConfigKey createKey(const FareCalcConfig* info,
                              const DateTime& startDate = DateTime::emptyDate(),
                              const DateTime& endDate = DateTime::emptyDate());

  void translateKey(const FareCalcConfigKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("USERAPPL", key._a);
    objectKey.setValue("USERAPPLCODE", key._b);
    objectKey.setValue("PSEUDOCITYCODE", key._c);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserterWithDateRange<FareCalcConfig, FareCalcConfigHistoricalDAO>(
               flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  virtual sfc::CompressedData* compress(const std::vector<FareCalcConfig*>* vect) const override;

  virtual std::vector<FareCalcConfig*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FareCalcConfigHistoricalDAO>;
  static DAOHelper<FareCalcConfigHistoricalDAO> _helper;
  FareCalcConfigHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FareCalcConfigKey, std::vector<FareCalcConfig*>, false>(
          cacheSize, cacheType, 3)
  {
  }
  std::vector<FareCalcConfig*>* create(FareCalcConfigKey key) override;
  void destroy(FareCalcConfigKey key, std::vector<FareCalcConfig*>* t) override;
  void load() override;

private:
  struct groupByKey;
  static FareCalcConfigHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
