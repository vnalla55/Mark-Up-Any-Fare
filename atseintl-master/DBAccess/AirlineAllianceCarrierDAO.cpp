// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------
#include "DBAccess/AirlineAllianceCarrierDAO.h"

#include "Common/Logger.h"
#include "DBAccess/AirlineAllianceCarrierInfo.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DAOUtils.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DBHistoryServer.h"
#include "DBAccess/Queries/QueryGetAirlineAllianceCarrier.h"

namespace tse
{
log4cxx::LoggerPtr
AirlineAllianceCarrierDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.AirlineAllianceCarrierDAO"));

AirlineAllianceCarrierDAO&
AirlineAllianceCarrierDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<AirlineAllianceCarrierInfo*>&
getAirlineAllianceCarrierData(const CarrierCode& carrierCode,
                              DeleteList& deleteList,
                              const DateTime& ticketDate,
                              bool isHistorical)
{
  if (isHistorical)
  {
    AirlineAllianceCarrierHistoricalDAO& dao = AirlineAllianceCarrierHistoricalDAO::instance();
    const std::vector<AirlineAllianceCarrierInfo*>& ret(
        dao.get(deleteList, carrierCode, ticketDate));

    return ret;
  }
  else
  {
    AirlineAllianceCarrierDAO& dao = AirlineAllianceCarrierDAO::instance();
    const std::vector<AirlineAllianceCarrierInfo*>& ret(
        dao.get(deleteList, carrierCode, ticketDate));

    return ret;
  }
}

const std::vector<AirlineAllianceCarrierInfo*>&
AirlineAllianceCarrierDAO::get(DeleteList& del,
                               const CarrierCode& carrierCode,
                               const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  CarrierKey key(carrierCode);
  DAOCache::pointer_type ptr(cache().get(key));
  del.copy(ptr);
  std::vector<AirlineAllianceCarrierInfo*>* ret = new std::vector<AirlineAllianceCarrierInfo*>;
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsEffectiveG<AirlineAllianceCarrierInfo>(ticketDate));
  while (i != ptr->end())
  {
    ret->push_back(*i);
    i = find_if(++i, ptr->end(), IsEffectiveG<AirlineAllianceCarrierInfo>(ticketDate));
  }
  del.adopt(ret);
  return *ret;
}

std::vector<AirlineAllianceCarrierInfo*>*
AirlineAllianceCarrierDAO::create(CarrierKey key)
{
  std::vector<AirlineAllianceCarrierInfo*>* ret = new std::vector<AirlineAllianceCarrierInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAirlineAllianceCarrier fnc(dbAdapter->getAdapter());
    fnc.findAirlineAllianceCarrierInfo(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in AirlineAllianceCarrierDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
AirlineAllianceCarrierDAO::destroy(CarrierKey key, std::vector<AirlineAllianceCarrierInfo*>* recs)
{
  destroyContainer(recs);
}

CarrierKey
AirlineAllianceCarrierDAO::createKey(AirlineAllianceCarrierInfo* info)
{
  return CarrierKey(info->carrier());
}

void
AirlineAllianceCarrierDAO::load()
{
  // not pre_loading

  StartupLoaderNoDB<AirlineAllianceCarrierInfo, AirlineAllianceCarrierDAO>();
}

size_t
AirlineAllianceCarrierDAO::invalidate(const ObjectKey& objectKey)
{
  return cache().clear();
}

std::string
AirlineAllianceCarrierDAO::_name("AirlineAllianceCarrier");
std::string
AirlineAllianceCarrierDAO::_cacheClass("Common");
DAOHelper<AirlineAllianceCarrierDAO>
AirlineAllianceCarrierDAO::_helper(_name);
AirlineAllianceCarrierDAO* AirlineAllianceCarrierDAO::_instance = nullptr;

///////////////////////////////////////////////////////
// Historical DAO Object
///////////////////////////////////////////////////////
log4cxx::LoggerPtr
AirlineAllianceCarrierHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.AirlineAllianceCarrierHistDAO"));
AirlineAllianceCarrierHistoricalDAO&
AirlineAllianceCarrierHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<AirlineAllianceCarrierInfo*>&
AirlineAllianceCarrierHistoricalDAO::get(DeleteList& del,
                                         const CarrierCode& carrierCode,
                                         const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  CarrierHistoricalKey key(carrierCode);
  DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<AirlineAllianceCarrierInfo*>* ret = new std::vector<AirlineAllianceCarrierInfo*>;
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(),
              ptr->end(),
              IsEffectiveHist<AirlineAllianceCarrierInfo>(ticketDate, ticketDate));
  while (i != ptr->end())
  {
    ret->push_back(*i);
    i = find_if(++i, ptr->end(), IsEffectiveH<AirlineAllianceCarrierInfo>(ticketDate, ticketDate));
  }
  del.adopt(ret);
  return *ret;
}

std::vector<AirlineAllianceCarrierInfo*>*
AirlineAllianceCarrierHistoricalDAO::create(CarrierHistoricalKey key)
{
  std::vector<AirlineAllianceCarrierInfo*>* ret = new std::vector<AirlineAllianceCarrierInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAirlineAllianceCarrierHistorical fnc(dbAdapter->getAdapter());
    fnc.findAirlineAllianceCarrierInfo(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in AirlineAllianceCarrierHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
AirlineAllianceCarrierHistoricalDAO::destroy(CarrierHistoricalKey key,
                                             std::vector<AirlineAllianceCarrierInfo*>* recs)
{
  std::vector<AirlineAllianceCarrierInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

size_t
AirlineAllianceCarrierHistoricalDAO::invalidate(const ObjectKey& objectKey)
{
  return cache().clear();
}

std::string
AirlineAllianceCarrierHistoricalDAO::_name("AirlineAllianceCarrierHistorical");
std::string
AirlineAllianceCarrierHistoricalDAO::_cacheClass("Common");
DAOHelper<AirlineAllianceCarrierHistoricalDAO>
AirlineAllianceCarrierHistoricalDAO::_helper(_name);
AirlineAllianceCarrierHistoricalDAO* AirlineAllianceCarrierHistoricalDAO::_instance = nullptr;
}
