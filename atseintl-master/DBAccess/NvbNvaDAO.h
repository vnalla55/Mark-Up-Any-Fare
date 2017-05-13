//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
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
class NvbNvaInfo;

typedef HashKey<VendorCode, CarrierCode, TariffNumber, RuleNumber> NvbNvaKey;
typedef HashKey<VendorCode, CarrierCode, TariffNumber, RuleNumber, DateTime, DateTime>
NvbNvaHistoricalKey;

class NvbNvaDAO : public DataAccessObject<NvbNvaKey, std::vector<NvbNvaInfo*> >
{
public:
  friend class DAOHelper<NvbNvaDAO>;

  static NvbNvaDAO& instance();

  const std::vector<NvbNvaInfo*>& get(DeleteList& del,
                                      const VendorCode& vendor,
                                      const CarrierCode& carrier,
                                      const TariffNumber& ruleTariff,
                                      const RuleNumber& rule,
                                      const DateTime& ticketDate);

  NvbNvaKey createKey(NvbNvaInfo* info);

  bool translateKey(const ObjectKey& objectKey, NvbNvaKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b) &&
               objectKey.getValue("RULETARIFF", key._c) && objectKey.getValue("RULE", key._d);
  }

  void translateKey(const NvbNvaKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("CARRIER", key._b);
    objectKey.setValue("RULETARIFF", key._c);
    objectKey.setValue("RULE", key._d);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<NvbNvaInfo, NvbNvaDAO>(flatKey, objectKey).success();
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

private:
  NvbNvaDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<NvbNvaKey, std::vector<NvbNvaInfo*> >(cacheSize, cacheType)
  {
  }

  std::vector<NvbNvaInfo*>* create(NvbNvaKey key) override;
  void destroy(NvbNvaKey key, std::vector<NvbNvaInfo*>* recs) override;
  void load() override;

  static std::string _name;
  static std::string _cacheClass;
  static DAOHelper<NvbNvaDAO> _helper;
  static NvbNvaDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class NvbNvaDAO

class NvbNvaHistoricalDAO
    : public HistoricalDataAccessObject<NvbNvaHistoricalKey, std::vector<NvbNvaInfo*> >
{
public:
  friend class DAOHelper<NvbNvaHistoricalDAO>;

  static NvbNvaHistoricalDAO& instance();

  const std::vector<NvbNvaInfo*>& get(DeleteList& del,
                                      const VendorCode& vendor,
                                      const CarrierCode& carrier,
                                      const TariffNumber& ruleTariff,
                                      const RuleNumber& rule,
                                      const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, NvbNvaHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b) &&
               objectKey.getValue("RULETARIFF", key._c) && objectKey.getValue("RULE", key._d) &&
               objectKey.getValue("STARTDATE", key._e) && objectKey.getValue("ENDDATE", key._f);
  }

  bool translateKey(const ObjectKey& objectKey,
                    NvbNvaHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._e, key._f, _cacheBy);

    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b) &&
               objectKey.getValue("RULETARIFF", key._c) && objectKey.getValue("RULE", key._d);
  }

  void setKeyDateRange(NvbNvaHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._e, key._f, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

private:
  NvbNvaHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<NvbNvaHistoricalKey, std::vector<NvbNvaInfo*> >(cacheSize,
                                                                                 cacheType)
  {
  }
  std::vector<NvbNvaInfo*>* create(NvbNvaHistoricalKey key) override;
  void destroy(NvbNvaHistoricalKey key, std::vector<NvbNvaInfo*>* t) override;

  static std::string _name;
  static std::string _cacheClass;
  static DAOHelper<NvbNvaHistoricalDAO> _helper;
  static NvbNvaHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class NvbNvaHistoricalDAO

} // namespace tse

