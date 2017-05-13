//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/TicketingFeesDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DAOUtils.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DBHistoryServer.h"
#include "DBAccess/Queries/QueryGetTicketingFees.h"
#include "DBAccess/TicketingFeesInfo.h"

namespace tse
{
log4cxx::LoggerPtr
TicketingFeesDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.TicketingFeesDAO"));

TicketingFeesDAO&
TicketingFeesDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<TicketingFeesInfo*>&
getTicketingFeesData(const VendorCode& vendor,
                     const CarrierCode& validatingCarrier,
                     const DateTime& date,
                     DeleteList& deleteList,
                     const DateTime& ticketDate,
                     bool isHistorical)
{
  if (isHistorical)
  {
    TicketingFeesHistoricalDAO& dao = TicketingFeesHistoricalDAO::instance();

    const std::vector<TicketingFeesInfo*>& ret =
        dao.get(deleteList, vendor, validatingCarrier, date, ticketDate);

    return ret;
  }
  else
  {
    TicketingFeesDAO& dao = TicketingFeesDAO::instance();

    const std::vector<TicketingFeesInfo*>& ret =
        dao.get(deleteList, vendor, validatingCarrier, date, ticketDate);

    return ret;
  }
}

const std::vector<TicketingFeesInfo*>&
TicketingFeesDAO::get(DeleteList& del,
                      const VendorCode& vendor,
                      const CarrierCode& validatingCarrier,
                      const DateTime& date,
                      const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  TicketingFeesKey key(vendor, validatingCarrier);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<TicketingFeesInfo*>* ret = new std::vector<TicketingFeesInfo*>;
  del.adopt(ret);

  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveG<TicketingFeesInfo>(date, ticketDate));

  return *ret;
}

std::vector<TicketingFeesInfo*>*
TicketingFeesDAO::create(TicketingFeesKey key)
{
  std::vector<TicketingFeesInfo*>* ret = new std::vector<TicketingFeesInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTicketingFees fnc(dbAdapter->getAdapter());
    fnc.findTicketingFeesInfo(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TicketingFeesDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
TicketingFeesDAO::destroy(TicketingFeesKey key, std::vector<TicketingFeesInfo*>* recs)
{
  destroyContainer(recs);
}

TicketingFeesKey
TicketingFeesDAO::createKey(TicketingFeesInfo* info)
{
  return TicketingFeesKey(info->vendor(), info->carrier());
}

void
TicketingFeesDAO::load()
{
  // not pre_loading

  StartupLoaderNoDB<TicketingFeesInfo, TicketingFeesDAO>();
}

std::string
TicketingFeesDAO::_name("TicketingFees");
std::string
TicketingFeesDAO::_cacheClass("Rules");
DAOHelper<TicketingFeesDAO>
TicketingFeesDAO::_helper(_name);
TicketingFeesDAO* TicketingFeesDAO::_instance = nullptr;

///////////////////////////////////////////////////////
// Historical DAO Object
///////////////////////////////////////////////////////
log4cxx::LoggerPtr
TicketingFeesHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.TicketingFeesHistDAO"));
TicketingFeesHistoricalDAO&
TicketingFeesHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<TicketingFeesInfo*>&
TicketingFeesHistoricalDAO::get(DeleteList& del,
                                const VendorCode& vendor,
                                const CarrierCode& carrier,
                                const DateTime& date,
                                const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  TicketingFeesHistoricalKey key(vendor, carrier);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<TicketingFeesInfo*>* ret = new std::vector<TicketingFeesInfo*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveH<TicketingFeesInfo>(date, ticketDate));

  return *ret;
}

std::vector<TicketingFeesInfo*>*
TicketingFeesHistoricalDAO::create(TicketingFeesHistoricalKey key)
{
  std::vector<TicketingFeesInfo*>* ret = new std::vector<TicketingFeesInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTicketingFeesHistorical fnc(dbAdapter->getAdapter());
    fnc.findTicketingFeesInfo(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TicketingFeesHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
TicketingFeesHistoricalDAO::destroy(TicketingFeesHistoricalKey key,
                                    std::vector<TicketingFeesInfo*>* recs)
{
  std::vector<TicketingFeesInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
TicketingFeesHistoricalDAO::_name("TicketingFeesHistorical");
std::string
TicketingFeesHistoricalDAO::_cacheClass("Rules");
DAOHelper<TicketingFeesHistoricalDAO>
TicketingFeesHistoricalDAO::_helper(_name);
TicketingFeesHistoricalDAO* TicketingFeesHistoricalDAO::_instance = nullptr;

} // namespace tse
