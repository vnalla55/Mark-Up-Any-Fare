//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/MarketCarriersDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/MarketCarrier.h"
#include "DBAccess/Queries/QueryGetMarketCarriers.h"

namespace tse
{
const std::vector<CarrierCode>&
getCarriersForMarketData(const LocCode& market1,
                         const LocCode& market2,
                         bool includeAddon,
                         const DateTime& date,
                         DeleteList& deleteList,
                         const DateTime& ticketDate,
                         bool isHistorical)
{
  MarketCarriersDAO& dao = MarketCarriersDAO::instance();
  if (market1 < market2)
  {
    return dao.getCarriers(deleteList, market1, market2);
  }
  else
  {
    return dao.getCarriers(deleteList, market2, market1);
  }
}

log4cxx::LoggerPtr
MarketCarriersDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.MarketCarriersDAO"));
MarketCarriersDAO&
MarketCarriersDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<CarrierCode>&
MarketCarriersDAO::getCarriers(DeleteList& del, const LocCode& market1, const LocCode& market2)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  MarketKey key(market1, market2);
  DAOCache::pointer_type ptr = cache().get(key);

  std::vector<CarrierCode>* cxrCodes = new std::vector<CarrierCode>;
  DAOCache::value_type::iterator i = ptr->begin();
  for (; i != ptr->end(); i++)
  {
    if (!Inhibit<MarketCarrier>(*i))
      cxrCodes->push_back((*i)->_carrier);
  }
  del.adopt(cxrCodes);
  return *cxrCodes;
}

std::vector<const tse::MarketCarrier*>*
MarketCarriersDAO::create(MarketKey key)
{
  std::vector<const MarketCarrier*>* ret = new std::vector<const MarketCarrier*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetMarketCarriers dmc(dbAdapter->getAdapter());
    dmc.findMarketCarriers(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in MarketCarriersDAO::create()");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

void
MarketCarriersDAO::destroy(MarketKey key, std::vector<const MarketCarrier*>* recs)
{
  std::vector<const MarketCarrier*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i; // lint !e605
  delete recs;
}

sfc::CompressedData*
MarketCarriersDAO::compress(const std::vector<const MarketCarrier*>* vect) const
{
  return compressVector(vect);
}

std::vector<const MarketCarrier*>*
MarketCarriersDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<const MarketCarrier>(compressed);
}

std::string
MarketCarriersDAO::_name("MarketCarriers");
std::string
MarketCarriersDAO::_cacheClass("Fares");
DAOHelper<MarketCarriersDAO>
MarketCarriersDAO::_helper(_name);
MarketCarriersDAO* MarketCarriersDAO::_instance = nullptr;

} // namespace tse
