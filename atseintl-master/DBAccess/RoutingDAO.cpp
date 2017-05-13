//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/RoutingDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetRouting.h"
#include "DBAccess/Routing.h"

namespace tse
{
log4cxx::LoggerPtr
RoutingDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.RoutingDAO"));
RoutingDAO&
RoutingDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<Routing*>&
getRoutingData(const VendorCode& vendor,
               const CarrierCode& carrier,
               const TariffNumber& routingTariff,
               const RoutingNumber& routingNumber,
               const DateTime& date,
               DeleteList& deleteList,
               const DateTime& ticketDate,
               bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    RoutingHistoricalDAO& dao = RoutingHistoricalDAO::instance();
    return dao.get(deleteList, vendor, carrier, routingTariff, routingNumber, date, ticketDate);
  }
  else
  {
    RoutingDAO& dao = RoutingDAO::instance();
    return dao.get(deleteList, vendor, carrier, routingTariff, routingNumber, date, ticketDate);
  }
}

const std::vector<Routing*>&
RoutingDAO::get(DeleteList& del,
                const VendorCode& vendor,
                const CarrierCode& carrier,
                const TariffNumber& routingTariff,
                const RoutingNumber& routingNumber,
                const DateTime& date,
                const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  RoutingKey key(vendor, carrier, routingTariff, routingNumber);
  DAOCache::pointer_type ptr = cache().get(key);
  /*
  del.copy(ptr);
  std::vector<Routing*>* ret = new std::vector<Routing*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret),
  IsNotEffectiveG<Routing>(date,ticketDate));
  return *ret;
  */
  return *(applyFilter(del, ptr, IsNotEffectiveG<Routing>(date, ticketDate)));
}

std::vector<Routing*>*
RoutingDAO::create(RoutingKey key)
{
  std::vector<Routing*>* ret = new std::vector<Routing*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetRouting rtg(dbAdapter->getAdapter());
    rtg.findRouting(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in RoutingDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

sfc::CompressedData*
RoutingDAO::compress(const std::vector<Routing*>* vect) const
{
  return compressVector(vect);
}

std::vector<Routing*>*
RoutingDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<Routing>(compressed);
}

void
RoutingDAO::destroy(RoutingKey key, std::vector<Routing*>* recs)
{
  std::vector<Routing*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

RoutingKey
RoutingDAO::createKey(Routing* info)
{
  return RoutingKey(info->vendor(), info->carrier(), info->routingTariff(), info->routing());
}

void
RoutingDAO::load()
{
  StartupLoaderNoDB<Routing, RoutingDAO>();
}

std::string
RoutingDAO::_name("Routing");
std::string
RoutingDAO::_cacheClass("Routing");
DAOHelper<RoutingDAO>
RoutingDAO::_helper(_name);
RoutingDAO* RoutingDAO::_instance = nullptr;

// Historical Stuff /////////////////////////////////////////////////////////
log4cxx::LoggerPtr
RoutingHistoricalDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.RoutingHistoricalDAO"));
RoutingHistoricalDAO&
RoutingHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<Routing*>&
RoutingHistoricalDAO::get(DeleteList& del,
                          const VendorCode& vendor,
                          const CarrierCode& carrier,
                          const TariffNumber& routingTariff,
                          const RoutingNumber& routingNumber,
                          const DateTime& date,
                          const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  RoutingHistoricalKey key(vendor, carrier, routingTariff, routingNumber);
  DAOUtils::getDateRange(ticketDate, key._e, key._f, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<Routing*>* ret = new std::vector<Routing*>;
  del.adopt(ret);
  remove_copy_if(
      ptr->begin(), ptr->end(), back_inserter(*ret), IsNotEffectiveHist<Routing>(date, ticketDate));
  return *ret;
}

std::vector<Routing*>*
RoutingHistoricalDAO::create(RoutingHistoricalKey key)
{
  std::vector<Routing*>* ret = new std::vector<Routing*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get("Historical", true);
  try
  {
    QueryGetRoutingHistorical rtg(dbAdapter->getAdapter());
    rtg.findRouting(*ret, key._a, key._b, key._c, key._d, key._e, key._f);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in RoutingHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
RoutingHistoricalDAO::destroy(RoutingHistoricalKey key, std::vector<Routing*>* recs)
{
  std::vector<Routing*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
RoutingHistoricalDAO::compress(const std::vector<Routing*>* vect) const
{
  return compressVector(vect);
}

std::vector<Routing*>*
RoutingHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<Routing>(compressed);
}

RoutingHistoricalKey
RoutingHistoricalDAO::createKey(const Routing* info,
                                const DateTime& startDate,
                                const DateTime& endDate)
{
  return RoutingHistoricalKey(
      info->vendor(), info->carrier(), info->routingTariff(), info->routing(), startDate, endDate);
}

void
RoutingHistoricalDAO::load()
{
  StartupLoaderNoDB<Routing, RoutingHistoricalDAO>();
}

std::string
RoutingHistoricalDAO::_name("RoutingHistorical");
std::string
RoutingHistoricalDAO::_cacheClass("Routing");
DAOHelper<RoutingHistoricalDAO>
RoutingHistoricalDAO::_helper(_name);
RoutingHistoricalDAO* RoutingHistoricalDAO::_instance = nullptr;

} // namespace tse
