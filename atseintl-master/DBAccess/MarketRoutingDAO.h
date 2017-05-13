//----------------------------------------------------------------------------
//  File:        MarketRoutingDAO.h
//  Created:     2009-01-01
//
//  Description: Market routing DAO class
//
//  Updates:
//
//  Copyright Sabre 2009
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/MarketRoutingInfo.h"

namespace tse
{
class MarketRouting;
class DeleteList;

class MarketRoutingDAO : public DataAccessObject<MarketRoutingKey, MarketRouting>
{
  friend class DAOHelper<MarketRoutingDAO>;

public:
  virtual ~MarketRoutingDAO();

  const std::string& cacheClass() override;

  void load() override;

  bool translateKey(const ObjectKey& objectKey, MarketRoutingKey& key) const override;

  const MarketRouting* get(DeleteList& del,
                           const VendorCode& vendor,
                           const CarrierCode& carrier,
                           const RoutingNumber& routing,
                           TariffNumber routingTariff);

  static MarketRoutingDAO& instance();

  static void checkForUpdates();

private:
  MarketRoutingDAO(int cacheSize = 0, const std::string& cacheType = "");

  MarketRouting* create(MarketRoutingKey key) override;

  void destroy(MarketRoutingKey key, MarketRouting* t) override;

  static log4cxx::LoggerPtr _logger;
  static DAOHelper<MarketRoutingDAO> _helper;
  static std::string _name;
  static std::string _cacheClass;
  static MarketRoutingDAO* _instance;
  static int32_t _latestGeneration;
  static std::string _dataDir;
};

} // namespace tse

