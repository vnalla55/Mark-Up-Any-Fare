//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/CabinDAO.h"

#include "Common/Logger.h"
#include "DBAccess/Cabin.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetCabin.h"

namespace tse
{
log4cxx::LoggerPtr
CabinDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.CabinDAO"));

CabinDAO&
CabinDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const Cabin*
getCabinData(const CarrierCode& carrier,
             const BookingCode& classOfService,
             const DateTime& date,
             DeleteList& deleteList,
             const DateTime& ticketDate,
             bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    CabinHistoricalDAO& dao = CabinHistoricalDAO::instance();
    return dao.get(deleteList, carrier, classOfService, date, ticketDate);
  }
  else
  {
    CabinDAO& dao = CabinDAO::instance();
    return dao.get(deleteList, carrier, classOfService, date, ticketDate);
  }
}
const Cabin*
CabinDAO::get(DeleteList& del,
              const CarrierCode& carrier,
              const BookingCode& classOfService,
              const DateTime& date,
              const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  IsEffectiveG<Cabin> dateFilter(date, ticketDate);
  DAOCache::pointer_type ptr = cache().get(CarrierKey(carrier));
  DAOCache::value_type::iterator i = std::find_if(ptr->begin(), ptr->end(), dateFilter);
  while (i != ptr->end())
  {
    if ((*i)->classOfService() == classOfService.c_str())
    {
      del.copy(ptr);
      return *i;
    }
    i = std::find_if(++i, ptr->end(), dateFilter);
  }
  ptr = cache().get(CarrierKey(""));
  i = std::find_if(ptr->begin(), ptr->end(), dateFilter);
  while (i != ptr->end())
  {
    if ((*i)->classOfService() == classOfService.c_str())
    {
      del.copy(ptr);
      return *i;
    }
    i = std::find_if(++i, ptr->end(), dateFilter);
  }
  return nullptr;
}

std::vector<Cabin*>*
CabinDAO::create(CarrierKey key)
{
  std::vector<Cabin*>* ret = new std::vector<Cabin*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetCabin cab(dbAdapter->getAdapter());
    cab.findCabin(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CabinDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
CabinDAO::destroy(CarrierKey key, std::vector<Cabin*>* recs)
{
  destroyContainer(recs);
}

void
CabinDAO::load()
{
  StartupLoader<QueryGetAllCabin, Cabin, CabinDAO>();
}

CarrierKey
CabinDAO::createKey(Cabin* info)
{
  return CarrierKey(info->carrier());
}

std::string
CabinDAO::_name("Cabin");
std::string
CabinDAO::_cacheClass("BookingCode");

DAOHelper<CabinDAO>
CabinDAO::_helper(_name);

CabinDAO* CabinDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: CabinHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
CabinHistoricalDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.CabinHistoricalDAO"));
CabinHistoricalDAO&
CabinHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const Cabin*
CabinHistoricalDAO::get(DeleteList& del,
                        const CarrierCode& carrier,
                        const BookingCode& classOfService,
                        const DateTime& date,
                        const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  IsEffectiveHist<Cabin> dateFilter(date, ticketDate);
  DAOCache::pointer_type ptr = cache().get(CarrierKey(carrier));
  DAOCache::value_type::iterator i = std::find_if(ptr->begin(), ptr->end(), dateFilter);
  while (i != ptr->end())
  {
    if ((*i)->classOfService() == classOfService.c_str())
    {
      del.copy(ptr);
      return *i;
    }
    i = std::find_if(++i, ptr->end(), dateFilter);
  }
  ptr = cache().get(CarrierKey(""));
  i = std::find_if(ptr->begin(), ptr->end(), dateFilter);
  while (i != ptr->end())
  {
    if ((*i)->classOfService() == classOfService.c_str())
    {
      del.copy(ptr);
      return *i;
    }
    i = std::find_if(++i, ptr->end(), dateFilter);
  }
  return nullptr;
}

std::vector<Cabin*>*
CabinHistoricalDAO::create(CarrierKey key)
{
  std::vector<Cabin*>* ret = new std::vector<Cabin*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetCabinHistorical cab(dbAdapter->getAdapter());
    cab.findCabin(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CabinHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

CarrierKey
CabinHistoricalDAO::createKey(Cabin* info, const DateTime& startDate, const DateTime& endDate)
{
  return CarrierKey(info->carrier());
}

void
CabinHistoricalDAO::load()
{
  StartupLoader<QueryGetAllCabinHistorical, Cabin, CabinHistoricalDAO>();
}

void
CabinHistoricalDAO::destroy(CarrierKey key, std::vector<Cabin*>* recs)
{
  destroyContainer(recs);
}

std::string
CabinHistoricalDAO::_name("CabinHistorical");
std::string
CabinHistoricalDAO::_cacheClass("BookingCode");
DAOHelper<CabinHistoricalDAO>
CabinHistoricalDAO::_helper(_name);
CabinHistoricalDAO* CabinHistoricalDAO::_instance = nullptr;

} // namespace tse
