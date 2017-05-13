//-------------------------------------------------------------------
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//-------------------------------------------------------------------

#include "DBAccess/FrequentFlyerStatusSegDAO.h"

#include "Common/Logger.h"
#include "DBAccess/Queries/QueryGetFrequentFlyerStatusSeg.h"

#include <cstdlib>

namespace tse
{
std::vector<const FreqFlyerStatusSeg*>
getFreqFlyerStatusSegsData(DeleteList& del,
                           const CarrierCode carrier,
                           const DateTime& date,
                           const DateTime& ticketDate,
                           bool isHistorical)
{
  if (isHistorical)
  {
    auto& dao = FrequentFlyerStatusSegHistoricalDAO::instance();
    return dao.get(del, carrier, date, ticketDate);
  }
  auto& dao = FrequentFlyerStatusSegDAO::instance();
  return dao.get(del, carrier, date, ticketDate);
}

Logger
FrequentFlyerStatusSegDAO::_logger("atseintl.DBAccess.FrequentFlyerStatusSegDAO");

std::string
FrequentFlyerStatusSegDAO::_name("FrequentFlyerStatusSeg");

std::string
FrequentFlyerStatusSegDAO::_cacheClass("Common");

DAOHelper<FrequentFlyerStatusSegDAO>
FrequentFlyerStatusSegDAO::_helper(_name);

FrequentFlyerStatusSegDAO*
FrequentFlyerStatusSegDAO::_instance(nullptr);

FrequentFlyerStatusSegDAO&
FrequentFlyerStatusSegDAO::instance()
{
  if (_instance == nullptr)
    _helper.init();
  return *_instance;
}

std::vector<const FreqFlyerStatusSeg*>
FrequentFlyerStatusSegDAO::get(DeleteList& del,
                               const CarrierCode carrier,
                               const DateTime& date,
                               const DateTime& ticketDate)
{
  FFStatusSegKey key(carrier);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<const FreqFlyerStatusSeg*> ret;
  std::copy_if(ptr->begin(),
               ptr->end(),
               std::back_inserter(ret),
               IsEffectiveG<FreqFlyerStatusSeg>(date, ticketDate));
  return ret;
}

std::vector<FreqFlyerStatusSeg*>*
FrequentFlyerStatusSegDAO::create(FFStatusSegKey key)
{
  std::vector<FreqFlyerStatusSeg*>* ret = new std::vector<FreqFlyerStatusSeg*>;

  try
  {
    DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
    DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
    QueryGetFrequentFlyerStatusSeg bo(dbAdapter->getAdapter());
    bo.findTierStatus(key._a, *ret);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FrequentFlyerStatusSegDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
FrequentFlyerStatusSegDAO::destroy(FFStatusSegKey, std::vector<FreqFlyerStatusSeg*>* status)
{
  destroyContainer(status);
}

size_t
FrequentFlyerStatusSegDAO::clear()
{
  size_t result(cache().clear());
  return result;
}

Logger
FrequentFlyerStatusSegHistoricalDAO::_logger("atseintl.DBAccess.FrequentFlyerStatusSegDAO");

std::string
FrequentFlyerStatusSegHistoricalDAO::_name("FrequentFlyerStatusSegHistorical");

std::string
FrequentFlyerStatusSegHistoricalDAO::_cacheClass("Common");

DAOHelper<FrequentFlyerStatusSegHistoricalDAO>
FrequentFlyerStatusSegHistoricalDAO::_helper(_name);

FrequentFlyerStatusSegHistoricalDAO*
FrequentFlyerStatusSegHistoricalDAO::_instance(nullptr);

FrequentFlyerStatusSegHistoricalDAO&
FrequentFlyerStatusSegHistoricalDAO::instance()
{
  if (_instance == nullptr)
    _helper.init();
  return *_instance;
}

std::vector<const FreqFlyerStatusSeg*>
FrequentFlyerStatusSegHistoricalDAO::get(DeleteList& del,
                                         const CarrierCode carrier,
                                         const DateTime& date,
                                         const DateTime& ticketDate)
{
  FFStatusHistoricalSegKey key(carrier, date, ticketDate);
  DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<const FreqFlyerStatusSeg*> ret;
  std::copy_if(ptr->begin(),
               ptr->end(),
               std::back_inserter(ret),
               IsEffectiveH<FreqFlyerStatusSeg>(date, ticketDate));
  return ret;
}

std::vector<FreqFlyerStatusSeg*>*
FrequentFlyerStatusSegHistoricalDAO::create(FFStatusHistoricalSegKey key)
{
  std::vector<FreqFlyerStatusSeg*>* ret = new std::vector<FreqFlyerStatusSeg*>;

  try
  {
    DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
    DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
    QueryGetFrequentFlyerStatusSegHistorical bo(dbAdapter->getAdapter());
    bo.findTierStatusSegs(key._a, key._b, key._c, *ret);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FrequentFlyerStatusSegHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
FrequentFlyerStatusSegHistoricalDAO::destroy(FFStatusHistoricalSegKey,
                                             std::vector<FreqFlyerStatusSeg*>* statuses)
{
  destroyContainer(statuses);
}


} // tse namespace
