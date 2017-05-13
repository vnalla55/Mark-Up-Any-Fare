//-------------------------------------------------------------------------------
// (C) 2014, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc. Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DBAccess/PassengerAirlineDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/HistoricalDataAccessObject.h"
#include "DBAccess/PassengerAirlineInfo.h"
#include "DBAccess/Queries/QueryGetPassengerAirline.h"

#include <algorithm>

namespace tse
{

log4cxx::LoggerPtr
PassengerAirlineDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.PassengerAirlineDAO"));

PassengerAirlineDAO&
PassengerAirlineDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const PassengerAirlineInfo*
PassengerAirlineDAO::get(DeleteList& del,
                         const CarrierCode& airlineCode,
                         const DateTime& travelDate,
                         const DateTime& ticketDate)
{
  _codeCoverageGetCallCount++;

  PassengerAirlineKey key(airlineCode);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  PassengerAirlineInfo* pai = nullptr;
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsEffectiveG<PassengerAirlineInfo>(travelDate, ticketDate));

  if (i != ptr->end())
    pai = *i;
  return pai;
}

std::vector<PassengerAirlineInfo*>*
PassengerAirlineDAO::create(PassengerAirlineKey key)
{
  std::vector<PassengerAirlineInfo*>* paiList = new std::vector<PassengerAirlineInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetPassengerAirline paiQuery(dbAdapter->getAdapter());
    paiQuery.findPassengerAirline(*paiList, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in PassengerAirlineDAO::create");
    destroyContainer(paiList);
    throw;
  }

  return paiList;
}

PassengerAirlineKey
PassengerAirlineDAO::createKey(const PassengerAirlineInfo* pai)
{
  return PassengerAirlineKey(pai->getAirlineCode());
}

bool
PassengerAirlineDAO::translateKey(const ObjectKey& objectKey,
                                           PassengerAirlineKey& paiKey) const
{
  paiKey.initialized = objectKey.getValue("CARRIER", paiKey._a);

  return paiKey.initialized;
}

void
PassengerAirlineDAO::translateKey(const PassengerAirlineKey& paiKey,
                                           ObjectKey& objectKey) const
{
  objectKey.setValue("CARRIER", paiKey._a);
}

bool
PassengerAirlineDAO::insertDummyObject(std::string& flatKey, ObjectKey& objectKey)
{
  return DummyObjectInserter<PassengerAirlineInfo, PassengerAirlineDAO>(
             flatKey, objectKey).success();
}

void
PassengerAirlineDAO::destroy(PassengerAirlineKey key, std::vector<PassengerAirlineInfo*>* paiList)
{
  destroyContainer(paiList);
}

void
PassengerAirlineDAO::load()
{
  StartupLoaderNoDB<PassengerAirlineInfo, PassengerAirlineDAO>();
}

size_t
PassengerAirlineDAO::clear()
{
  size_t result(cache().clear());
  LOG4CXX_ERROR(_logger, "PassengerAirline cache cleared");
  return result;
}

std::string
PassengerAirlineDAO::_name("PassengerAirline");
std::string
PassengerAirlineDAO::_cacheClass("Common");
DAOHelper<PassengerAirlineDAO>
PassengerAirlineDAO::_helper(_name);
PassengerAirlineDAO* PassengerAirlineDAO::_instance = nullptr;

//-----------------------------------------------------------------------------
// HISTORICAL
//-----------------------------------------------------------------------------

log4cxx::LoggerPtr
PassengerAirlineHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.PassengerAirlineHistoricalDAO"));

PassengerAirlineHistoricalDAO&
PassengerAirlineHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const PassengerAirlineInfo*
PassengerAirlineHistoricalDAO::get(DeleteList& del,
                                   const CarrierCode& airlineCode,
                                   const DateTime& travelDate,
                                   const DateTime& ticketDate)
{
  _codeCoverageGetCallCount++;

  PassengerAirlineHistoricalKey key(airlineCode);
  DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  PassengerAirlineInfo* pai = nullptr;
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsEffectiveH<PassengerAirlineInfo>(travelDate, ticketDate));

  if (i != ptr->end())
    pai = *i;
  return pai;
}

std::vector<PassengerAirlineInfo*>*
PassengerAirlineHistoricalDAO::create(PassengerAirlineHistoricalKey key)
{
  std::vector<PassengerAirlineInfo*>* paiList = new std::vector<PassengerAirlineInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetPassengerAirlineHistorical paiQuery(dbAdapter->getAdapter());
    paiQuery.findPassengerAirline(*paiList, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in PassengerAirlineHistoricalDAO::create");
    destroyContainer(paiList);
    throw;
  }

  return paiList;
}

bool
PassengerAirlineHistoricalDAO::translateKey(const ObjectKey& objectKey,
                                            PassengerAirlineHistoricalKey& paiKey) const
{
  paiKey.initialized = objectKey.getValue("CARRIER", paiKey._a);

  return paiKey.initialized;
}

void
PassengerAirlineHistoricalDAO::translateKey(const PassengerAirlineHistoricalKey& paiKey,
                                            ObjectKey& objectKey) const
{
  objectKey.setValue("CARRIER", paiKey._a);
}

bool
PassengerAirlineHistoricalDAO::translateKey(const ObjectKey& objectKey,
                                            PassengerAirlineHistoricalKey& paiKey,
                                            const DateTime ticketDate) const
{
  DAOUtils::getDateRange(ticketDate, paiKey._b, paiKey._c, _cacheBy);
  return translateKey(objectKey, paiKey);
}

void
PassengerAirlineHistoricalDAO::destroy(PassengerAirlineHistoricalKey key,
                                       std::vector<PassengerAirlineInfo*>* paiList)
{
  destroyContainer(paiList);
}

std::string
PassengerAirlineHistoricalDAO::_name("PassengerAirlineHistorical");
std::string
PassengerAirlineHistoricalDAO::_cacheClass("Common");
DAOHelper<PassengerAirlineHistoricalDAO>
PassengerAirlineHistoricalDAO::_helper(_name);
PassengerAirlineHistoricalDAO* PassengerAirlineHistoricalDAO::_instance = nullptr;

}
