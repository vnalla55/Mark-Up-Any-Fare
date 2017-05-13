//-------------------------------------------------------------------------------
// Copyright 2004, 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "Common/TseCodeTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/HistoricalDataAccessObject.h"

namespace tse
{
class RoutingKeyInfo;
class DeleteList;

typedef HashKey<LocCode, LocCode, CarrierCode> RoutingForMarketKey;

class RoutingForMarketDAO
    : public DataAccessObject<RoutingForMarketKey, std::vector<RoutingKeyInfo*> >
{
public:
  static RoutingForMarketDAO& instance();

  const std::vector<RoutingKeyInfo*>& get(DeleteList& del,
                                          const LocCode& market1,
                                          const LocCode& market2,
                                          const CarrierCode& carrier,
                                          const DateTime& ticketDatel);

  bool translateKey(const ObjectKey& objectKey, RoutingForMarketKey& key) const override
  {
    return key.initialized = objectKey.getValue("MARKET1", key._a) &&
                             objectKey.getValue("MARKET2", key._b) &&
                             objectKey.getValue("CARRIER", key._c);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<RoutingForMarketDAO>;
  static DAOHelper<RoutingForMarketDAO> _helper;
  RoutingForMarketDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<RoutingForMarketKey, std::vector<RoutingKeyInfo*> >(cacheSize, cacheType)
  {
  }
  std::vector<RoutingKeyInfo*>* create(RoutingForMarketKey key) override;
  void destroy(RoutingForMarketKey key, std::vector<RoutingKeyInfo*>* t) override;

private:
  static RoutingForMarketDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class RoutingForMarketDAO

// Historical Stuff
// //////////////////////////////////////////////////////////////////////////////////////////
typedef HashKey<LocCode, LocCode, CarrierCode, DateTime, DateTime> RoutingForMarketHistoricalKey;

class RoutingForMarketHistoricalDAO
    : public HistoricalDataAccessObject<RoutingForMarketHistoricalKey,
                                        std::vector<RoutingKeyInfo*> >
{
public:
  static RoutingForMarketHistoricalDAO& instance();

  const std::vector<RoutingKeyInfo*>& get(DeleteList& del,
                                          const LocCode& market1,
                                          const LocCode& market2,
                                          const CarrierCode& carrier,
                                          const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, RoutingForMarketHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("MARKET1", key._a) && objectKey.getValue("MARKET2", key._b) &&
               objectKey.getValue("CARRIER", key._c) && objectKey.getValue("STARTDATE", key._d) &&
               objectKey.getValue("ENDDATE", key._e);
  }

  bool translateKey(const ObjectKey& objectKey,
                    RoutingForMarketHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
    return key.initialized = objectKey.getValue("MARKET1", key._a) &&
                             objectKey.getValue("MARKET2", key._b) &&
                             objectKey.getValue("CARRIER", key._c);
  }

  void setKeyDateRange(RoutingForMarketHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<RoutingForMarketHistoricalDAO>;
  static DAOHelper<RoutingForMarketHistoricalDAO> _helper;

  RoutingForMarketHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<RoutingForMarketHistoricalKey, std::vector<RoutingKeyInfo*> >(
          cacheSize, cacheType)
  {
  }

  std::vector<RoutingKeyInfo*>* create(RoutingForMarketHistoricalKey key) override;

  void destroy(RoutingForMarketHistoricalKey key, std::vector<RoutingKeyInfo*>* t) override;

private:
  static RoutingForMarketHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class RoutingForMarketHistoricalDAO
} // namespace tse
