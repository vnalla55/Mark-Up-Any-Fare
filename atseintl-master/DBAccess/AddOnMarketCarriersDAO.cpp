//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/AddOnMarketCarriersDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/MarketCarrier.h"
#include "DBAccess/Queries/QueryGetAddonMarketCarriers.h"

namespace tse
{
log4cxx::LoggerPtr
AddOnMarketCarriersDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.AddOnMarketCarriersDAO"));

AddOnMarketCarriersDAO&
AddOnMarketCarriersDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<CarrierCode>&
getAddOnCarriersForMarketData(const LocCode& market1,
                              const LocCode& market2,
                              const DateTime& date,
                              DeleteList& deleteList,
                              const DateTime& ticketDate,
                              bool isHistorical)
{
  if (isHistorical)
  {
    AddOnMarketCarriersHistoricalDAO& dao = AddOnMarketCarriersHistoricalDAO::instance();
    return dao.getCarriers(deleteList, market1, market2, date, ticketDate);
  }
  else
  {
    AddOnMarketCarriersDAO& dao = AddOnMarketCarriersDAO::instance();
    return dao.getCarriers(deleteList, market1, market2, date, ticketDate);
  }
}

const std::vector<CarrierCode>&
AddOnMarketCarriersDAO::getCarriers(DeleteList& del,
                                    const LocCode& market1,
                                    const LocCode& market2,
                                    const DateTime& date,
                                    const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  AddOnMarketKey key(market1, market2);
  DAOCache::pointer_type ptr = cache().get(key);

  std::vector<CarrierCode>* cxrCodes = new std::vector<CarrierCode>;
  DAOCache::value_type::iterator i =
      std::find_if(ptr->begin(), ptr->end(), IsEffectiveG<MarketCarrier>(date, ticketDate));
  while (i != ptr->end())
  {
    if (!Inhibit<MarketCarrier>(*i))
      cxrCodes->push_back((*i)->_carrier);
    i = std::find_if(++i, ptr->end(), IsEffectiveG<MarketCarrier>(date, ticketDate));
  }

  del.adopt(cxrCodes);
  return *cxrCodes;
}

std::vector<const tse::MarketCarrier*>*
AddOnMarketCarriersDAO::create(AddOnMarketKey key)
{
  std::vector<const MarketCarrier*>* ret = new std::vector<const MarketCarrier*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAddonMarketCarriers amc(dbAdapter->getAdapter());
    amc.findAddOnMarketCarriers(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in AddOnMarketCarriersDAO::create()");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
AddOnMarketCarriersDAO::destroy(AddOnMarketKey key, std::vector<const MarketCarrier*>* recs)
{
  std::vector<const MarketCarrier*>::iterator i;
  for (i = recs->begin(); i != recs->end(); ++i)
    delete *i; // lint !e605
  delete recs;
}

std::string
AddOnMarketCarriersDAO::_name("AddOnMarketCarriers");
std::string
AddOnMarketCarriersDAO::_cacheClass("Fares");
DAOHelper<AddOnMarketCarriersDAO>
AddOnMarketCarriersDAO::_helper(_name);
AddOnMarketCarriersDAO* AddOnMarketCarriersDAO::_instance = nullptr;

AddOnMarketKey
AddOnMarketCarriersDAO::createKey(const MarketCarrier* info)
{
  return AddOnMarketKey(info->market1(), info->market2());
}

void
AddOnMarketCarriersDAO::load()
{
  StartupLoaderNoDB<MarketCarrier, AddOnMarketCarriersDAO>();
}

// Historical Stuff ///////////////////////////////////////////////////////////////////////////
log4cxx::LoggerPtr
AddOnMarketCarriersHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.AddOnMarketCarriersHistDAO"));

AddOnMarketCarriersHistoricalDAO&
AddOnMarketCarriersHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<CarrierCode>&
AddOnMarketCarriersHistoricalDAO::getCarriers(DeleteList& del,
                                              const LocCode& market1,
                                              const LocCode& market2,
                                              const DateTime& date,
                                              const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  AddOnMarketHistoricalKey key(market1, market2);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);

  std::vector<CarrierCode>* cxrCodes = new std::vector<CarrierCode>;
  DAOCache::value_type::iterator i =
      std::find_if(ptr->begin(), ptr->end(), IsEffectiveHist<MarketCarrier>(date, ticketDate));
  while (i != ptr->end())
  {
    if (!Inhibit<MarketCarrier>(*i))
      cxrCodes->push_back((*i)->_carrier);
    i = std::find_if(++i, ptr->end(), IsEffectiveHist<MarketCarrier>(date, ticketDate));
  }

  del.adopt(cxrCodes);
  return *cxrCodes;
}

std::vector<const tse::MarketCarrier*>*
AddOnMarketCarriersHistoricalDAO::create(AddOnMarketHistoricalKey key)
{
  std::vector<const MarketCarrier*>* ret = new std::vector<const MarketCarrier*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get("Historical", true);
  try
  {
    QueryGetAddonMarketCarriersHistorical amc(dbAdapter->getAdapter());
    amc.findAddOnMarketCarriers(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in AddOnMarketCarriersHistoricalDAO::create()");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
AddOnMarketCarriersHistoricalDAO::destroy(AddOnMarketHistoricalKey key,
                                          std::vector<const MarketCarrier*>* recs)
{
  std::vector<const MarketCarrier*>::iterator i;
  for (i = recs->begin(); i != recs->end(); ++i)
    delete *i;
  delete recs;
}

std::string
AddOnMarketCarriersHistoricalDAO::_name("AddOnMarketCarriersHistorical");
std::string
AddOnMarketCarriersHistoricalDAO::_cacheClass("Fares");
DAOHelper<AddOnMarketCarriersHistoricalDAO>
AddOnMarketCarriersHistoricalDAO::_helper(_name);
AddOnMarketCarriersHistoricalDAO* AddOnMarketCarriersHistoricalDAO::_instance = nullptr;

} // namespace tse
