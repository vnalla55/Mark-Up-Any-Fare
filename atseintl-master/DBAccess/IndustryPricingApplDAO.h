//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
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
class IndustryPricingAppl;
class DeleteList;

class IndustryPricingApplDAO
    : public DataAccessObject<CarrierKey, std::vector<const IndustryPricingAppl*>, false>
{
public:
  static IndustryPricingApplDAO& instance();
  const std::vector<const IndustryPricingAppl*>& get(DeleteList& del,
                                                     const CarrierCode& carrier,
                                                     const GlobalDirection& globalDir,
                                                     const DateTime& date,
                                                     const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, CarrierKey& key) const override
  {
    return key.initialized = objectKey.getValue("CARRIER", key._a);
  }

  CarrierKey createKey(const IndustryPricingAppl* info);

  void translateKey(const CarrierKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CARRIER", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<IndustryPricingAppl, IndustryPricingApplDAO>(flatKey, objectKey)
        .success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<IndustryPricingApplDAO>;
  static DAOHelper<IndustryPricingApplDAO> _helper;
  IndustryPricingApplDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<CarrierKey, std::vector<const IndustryPricingAppl*>, false>(cacheSize,
                                                                                   cacheType)
  {
  }

  std::vector<const IndustryPricingAppl*>* create(CarrierKey key) override;
  void destroy(CarrierKey key, std::vector<const IndustryPricingAppl*>* recs) override;

  virtual void load() override;

private:
  static IndustryPricingApplDAO* _instance;
  struct IsNotGlobalDir;
  static log4cxx::LoggerPtr _logger;
};
// --------------------------------------------------
// Historical DAO: IndustryPricingApplHistoricalDAO
// --------------------------------------------------
class IndustryPricingApplHistoricalDAO
    : public HistoricalDataAccessObject<CarrierCode, std::vector<IndustryPricingAppl*>, false>
{
public:
  static IndustryPricingApplHistoricalDAO& instance();
  const std::vector<const IndustryPricingAppl*>& get(DeleteList& del,
                                                     const CarrierCode& carrier,
                                                     const GlobalDirection& globalDir,
                                                     const DateTime& date,
                                                     const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, CarrierCode& key) const override
  {
    return objectKey.getValue("CARRIER", key);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<IndustryPricingApplHistoricalDAO>;
  static DAOHelper<IndustryPricingApplHistoricalDAO> _helper;
  IndustryPricingApplHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<CarrierCode, std::vector<IndustryPricingAppl*>, false>(cacheSize,
                                                                                        cacheType)
  {
  }

  std::vector<IndustryPricingAppl*>* create(CarrierCode key) override;
  void destroy(CarrierCode key, std::vector<IndustryPricingAppl*>* recs) override;

  virtual void load() override;

private:
  static IndustryPricingApplHistoricalDAO* _instance;
  struct IsNotGlobalDir;
  struct groupByKey;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
