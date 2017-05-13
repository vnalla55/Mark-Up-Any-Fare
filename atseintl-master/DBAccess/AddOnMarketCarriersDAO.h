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
class MarketCarrier;
class DeleteList;

typedef HashKey<LocCode, LocCode> AddOnMarketKey;

class AddOnMarketCarriersDAO
    : public DataAccessObject<AddOnMarketKey, std::vector<const MarketCarrier*> >
{
public:
  static AddOnMarketCarriersDAO& instance();

  const std::vector<CarrierCode>& getCarriers(DeleteList& del,
                                              const LocCode& market1,
                                              const LocCode& market2,
                                              const DateTime& date,
                                              const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, AddOnMarketKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("MARKET1", key._a) && objectKey.getValue("MARKET2", key._b);
  }

  AddOnMarketKey createKey(const MarketCarrier* info);

  void translateKey(const AddOnMarketKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("MARKET1", key._a);
    objectKey.setValue("MARKET2", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<MarketCarrier, AddOnMarketCarriersDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<AddOnMarketCarriersDAO>;
  static DAOHelper<AddOnMarketCarriersDAO> _helper;
  AddOnMarketCarriersDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<AddOnMarketKey, std::vector<const MarketCarrier*> >(cacheSize, cacheType)
  {
  }
  std::vector<const tse::MarketCarrier*>* create(AddOnMarketKey key) override;
  void destroy(AddOnMarketKey key, std::vector<const tse::MarketCarrier*>* t) override;
  void load() override;

private:
  static AddOnMarketCarriersDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class AddOnMarketCarriersDAO

// Historical Stuff
// /////////////////////////////////////////////////////////////////////////////////
typedef HashKey<LocCode, LocCode, DateTime, DateTime> AddOnMarketHistoricalKey;

class AddOnMarketCarriersHistoricalDAO
    : public HistoricalDataAccessObject<AddOnMarketHistoricalKey,
                                        std::vector<const MarketCarrier*> >
{
public:
  static AddOnMarketCarriersHistoricalDAO& instance();

  const std::vector<CarrierCode>& getCarriers(DeleteList& del,
                                              const LocCode& market1,
                                              const LocCode& market2,
                                              const DateTime& date,
                                              const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, AddOnMarketHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._c);
    objectKey.getValue("ENDDATE", key._d);
    return key.initialized = objectKey.getValue("MARKET1", key._a)
                             && objectKey.getValue("MARKET2", key._b);
  }

  bool translateKey(const ObjectKey& objectKey,
                    AddOnMarketHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("MARKET1", key._a) && objectKey.getValue("MARKET2", key._b);
  }

  void setKeyDateRange(AddOnMarketHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<AddOnMarketCarriersHistoricalDAO>;
  static DAOHelper<AddOnMarketCarriersHistoricalDAO> _helper;
  AddOnMarketCarriersHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<AddOnMarketHistoricalKey, std::vector<const MarketCarrier*> >(
          cacheSize, cacheType)
  {
  }
  std::vector<const tse::MarketCarrier*>* create(AddOnMarketHistoricalKey key) override;
  void destroy(AddOnMarketHistoricalKey key, std::vector<const tse::MarketCarrier*>* t) override;

private:
  static AddOnMarketCarriersHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class AddOnMarketCarriersHistoricalDAO

} // namespace tse
