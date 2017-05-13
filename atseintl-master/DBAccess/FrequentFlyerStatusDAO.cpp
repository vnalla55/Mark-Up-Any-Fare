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

#include "DBAccess/FrequentFlyerStatusDAO.h"

#include "Common/Logger.h"
#include "DBAccess/FreqFlyerStatus.h"
#include "DBAccess/HistoricalDataAccessObject.h"
#include "DBAccess/Queries/QueryGetFrequentFlyerStatus.h"

#include <cstdlib>

namespace tse
{
std::vector<const FreqFlyerStatus*>
getFreqFlyerStatusesData(DeleteList& del,
                         const CarrierCode carrier,
                         const DateTime& date,
                         const DateTime& ticketDate,
                         const bool useHistorical)
{
  if (useHistorical)
    return FrequentFlyerStatusHistoricalDAO::instance().get(del, carrier, date, ticketDate);
  return FrequentFlyerStatusDAO::instance().get(del, carrier);
}

Logger
FrequentFlyerStatusDAO::_logger("atseintl.DBAccess.FrequentFlyerStatusDAO");

const std::string
FrequentFlyerStatusDAO::_name("FrequentFlyerStatus");

const std::string
FrequentFlyerStatusDAO::_cacheClass("Common");

DAOHelper<FrequentFlyerStatusDAO>
FrequentFlyerStatusDAO::_helper(_name);

FrequentFlyerStatusDAO*
FrequentFlyerStatusDAO::_instance(nullptr);

FrequentFlyerStatusDAO&
FrequentFlyerStatusDAO::instance()
{
  if (_instance == nullptr)
    _helper.init();
  return *_instance;
}

std::vector<const FreqFlyerStatus*>
FrequentFlyerStatusDAO::get(DeleteList& del,
                            const CarrierCode carrier)
{
  const FFStatusKey key(carrier);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  const DateTime current = DateTime::localTime();
  std::vector<const FreqFlyerStatus*> ret;
  std::copy_if(ptr->begin(),
               ptr->end(),
               std::back_inserter(ret),
               IsEffectiveG<FreqFlyerStatus>(current, current));
  return ret;
}

std::vector<FreqFlyerStatus*>*
FrequentFlyerStatusDAO::create(FFStatusKey key)
{
  try
  {
    DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
    DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
    QueryGetFrequentFlyerStatus bo(dbAdapter->getAdapter());
    return new std::vector<FreqFlyerStatus*>(std::move(bo.findStatus(key._a)));
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FrequentFlyerStatusDAO::create");
    throw;
  }
}

void
FrequentFlyerStatusDAO::destroy(FFStatusKey, std::vector<FreqFlyerStatus*>* status)
{
  destroyContainer(status);
}

size_t
FrequentFlyerStatusDAO::clear()
{
  size_t result(cache().clear());
  return result;
}

Logger
FrequentFlyerStatusHistoricalDAO::_logger("atseintl.DBAccess.FrequentFlyerStatusDAO");

std::string
FrequentFlyerStatusHistoricalDAO::_name("FrequentFlyerStatusHistorical");

std::string
FrequentFlyerStatusHistoricalDAO::_cacheClass("Common");

DAOHelper<FrequentFlyerStatusHistoricalDAO>
FrequentFlyerStatusHistoricalDAO::_helper(_name);

FrequentFlyerStatusHistoricalDAO*
FrequentFlyerStatusHistoricalDAO::_instance(nullptr);

FrequentFlyerStatusHistoricalDAO&
FrequentFlyerStatusHistoricalDAO::instance()
{
  if (_instance == nullptr)
    _helper.init();
  return *_instance;
}

std::vector<const FreqFlyerStatus*>
FrequentFlyerStatusHistoricalDAO::get(DeleteList& del,
                                      const CarrierCode carrier,
                                      const DateTime& date,
                                      const DateTime& ticketDate)
{
  const FFStatusHistoricalKey key(carrier, date, ticketDate);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<const FreqFlyerStatus*> ret;
  std::copy_if(ptr->begin(),
               ptr->end(),
               std::back_inserter(ret),
               IsEffectiveH<FreqFlyerStatus>(date, ticketDate));
  return ret;
}

std::vector<FreqFlyerStatus*>*
FrequentFlyerStatusHistoricalDAO::create(FFStatusHistoricalKey key)
{
  try
  {
    DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
    DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
    QueryGetFrequentFlyerStatusHistorical bo(dbAdapter->getAdapter());
    return new std::vector<FreqFlyerStatus*>(std::move(bo.findStatus(key._a, key._b, key._c)));
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FrequentFlyerStatusDAO::create");
    throw;
  }
}

void
FrequentFlyerStatusHistoricalDAO::destroy(FFStatusHistoricalKey,
                                          std::vector<FreqFlyerStatus*>* statuses)
{
  destroyContainer(statuses);
}

} // tse namespace
