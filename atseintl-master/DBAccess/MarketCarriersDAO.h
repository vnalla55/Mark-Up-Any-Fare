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

typedef HashKey<LocCode, LocCode> MarketKey;

class MarketCarriersDAO : public DataAccessObject<MarketKey, std::vector<const MarketCarrier*> >
{
public:
  static MarketCarriersDAO& instance();

  const std::vector<CarrierCode>&
  getCarriers(DeleteList& del, const LocCode& market1, const LocCode& market2);

  bool translateKey(const ObjectKey& objectKey, MarketKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("MARKET1", key._a) && objectKey.getValue("MARKET2", key._b);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData*
  compress(const std::vector<const MarketCarrier*>* vect) const override;

  virtual std::vector<const MarketCarrier*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<MarketCarriersDAO>;
  static DAOHelper<MarketCarriersDAO> _helper;
  MarketCarriersDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<MarketKey, std::vector<const MarketCarrier*> >(cacheSize, cacheType)
  {
  }
  std::vector<const tse::MarketCarrier*>* create(MarketKey key) override;
  void destroy(MarketKey key, std::vector<const tse::MarketCarrier*>* t) override;

private:
  static MarketCarriersDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

} // namespace tse
