//------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//------------------------------------------------------------------------------
//
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/DeleteList.h"

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class FareDisplayWeb;
class DeleteList;

class FareDisplayWebDAO : public DataAccessObject<CarrierKey, std::vector<FareDisplayWeb*>, false>
{
public:
  static FareDisplayWebDAO& instance();

  const std::vector<FareDisplayWeb*>& get(DeleteList& del, const CarrierCode& carrier);

  const std::set<std::pair<PaxTypeCode, VendorCode> >&
  getPsgForCxr(DeleteList& del, const CarrierCode& carrier);

  const std::vector<FareDisplayWeb*>& get(DeleteList& del,
                                          const Indicator& dispInd,
                                          const VendorCode& vendor,
                                          const CarrierCode& carrier,
                                          const TariffNumber& ruleTariff,
                                          const RuleNumber& rule,
                                          const PaxTypeCode& paxTypeCode);

  bool translateKey(const ObjectKey& objectKey, CarrierKey& key) const override
  {
    return key.initialized = objectKey.getValue("CARRIER", key._a);
  }

  CarrierKey createKey(FareDisplayWeb* info);

  void translateKey(const CarrierKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CARRIER", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<FareDisplayWeb, FareDisplayWebDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<FareDisplayWebDAO>;

  static DAOHelper<FareDisplayWebDAO> _helper;

  FareDisplayWebDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<CarrierKey, std::vector<FareDisplayWeb*>, false>(cacheSize, cacheType, 3)
  {
  }

  std::vector<FareDisplayWeb*>* create(CarrierKey key) override;
  void destroy(CarrierKey key, std::vector<FareDisplayWeb*>* recs) override;

  virtual void load() override;

private:
  static FareDisplayWebDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

} // namespace tse

