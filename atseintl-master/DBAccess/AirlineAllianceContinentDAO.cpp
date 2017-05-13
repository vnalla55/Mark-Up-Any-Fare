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
#include "DBAccess/AirlineAllianceContinentDAO.h"

#include "Common/Logger.h"
#include "DBAccess/AirlineAllianceContinentInfo.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DAOUtils.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DBHistoryServer.h"
#include "DBAccess/Queries/QueryGetAirlineAllianceContinent.h"

namespace tse
{
log4cxx::LoggerPtr
AirlineAllianceContinentDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.AirlineAllianceContinentDAO"));

AirlineAllianceContinentDAO&
AirlineAllianceContinentDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<AirlineAllianceContinentInfo*>&
getAirlineAllianceContinentData(const GenericAllianceCode& genericAllianceCode,
                                DeleteList& deleteList,
                                const DateTime& ticketDate,
                                bool isHistorical,
                                bool reduceTemporaryVectorsFallback)
{
  if (isHistorical)
  {
    AirlineAllianceContinentHistoricalDAO& dao = AirlineAllianceContinentHistoricalDAO::instance();
    const std::vector<AirlineAllianceContinentInfo*>& ret(
        dao.get(deleteList, genericAllianceCode, ticketDate, reduceTemporaryVectorsFallback));

    return ret;
  }
  else
  {
    AirlineAllianceContinentDAO& dao = AirlineAllianceContinentDAO::instance();
    const std::vector<AirlineAllianceContinentInfo*>& ret(
        dao.get(deleteList, genericAllianceCode, ticketDate, reduceTemporaryVectorsFallback));

    return ret;
  }
}

const std::vector<AirlineAllianceContinentInfo*>&
AirlineAllianceContinentDAO::get(DeleteList& del,
                                 const GenericAllianceCode& genericAllianceCode,
                                 const DateTime& ticketDate,
                                 bool reduceTemporaryVectorsFallback)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  GenericAllianceCodeKey key(genericAllianceCode);
  DAOCache::pointer_type ptr(cache().get(key));
  del.copy(ptr);

  if (!reduceTemporaryVectorsFallback)
  {
    return *ptr;
  }
  else
  {
    std::vector<AirlineAllianceContinentInfo*>* ret =
        new std::vector<AirlineAllianceContinentInfo*>;
    DAOCache::value_type::iterator i =
        find_if(ptr->begin(), ptr->end(), IsEffectiveG<AirlineAllianceContinentInfo>(ticketDate));
    while (i != ptr->end())
    {
      ret->push_back(*i);
      i = find_if(++i, ptr->end(), IsEffectiveG<AirlineAllianceContinentInfo>(ticketDate));
    }
    del.adopt(ret);
    return *ret;
  }
}

std::vector<AirlineAllianceContinentInfo*>*
AirlineAllianceContinentDAO::create(GenericAllianceCodeKey key)
{
  std::vector<AirlineAllianceContinentInfo*>* ret = new std::vector<AirlineAllianceContinentInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAirlineAllianceContinent fnc(dbAdapter->getAdapter());
    fnc.findAirlineAllianceContinentInfo(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in AirlineAllianceContinentDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
AirlineAllianceContinentDAO::destroy(GenericAllianceCodeKey key,
                                     std::vector<AirlineAllianceContinentInfo*>* recs)
{
  destroyContainer(recs);
}

GenericAllianceCodeKey
AirlineAllianceContinentDAO::createKey(AirlineAllianceContinentInfo* info)
{
  return GenericAllianceCodeKey(info->genericAllianceCode());
}

void
AirlineAllianceContinentDAO::load()
{
  // not pre_loading

  StartupLoaderNoDB<AirlineAllianceContinentInfo, AirlineAllianceContinentDAO>();
}

size_t
AirlineAllianceContinentDAO::invalidate(const ObjectKey& objectKey)
{
  return cache().clear();
}

std::string
AirlineAllianceContinentDAO::_name("AirlineAllianceContinent");
std::string
AirlineAllianceContinentDAO::_cacheClass("Common");
DAOHelper<AirlineAllianceContinentDAO>
AirlineAllianceContinentDAO::_helper(_name);
AirlineAllianceContinentDAO* AirlineAllianceContinentDAO::_instance = nullptr;

///////////////////////////////////////////////////////
// Historical DAO Object
///////////////////////////////////////////////////////
log4cxx::LoggerPtr
AirlineAllianceContinentHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.AirlineAllianceContinentHistDAO"));
AirlineAllianceContinentHistoricalDAO&
AirlineAllianceContinentHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<AirlineAllianceContinentInfo*>&
AirlineAllianceContinentHistoricalDAO::get(DeleteList& del,
                                           const GenericAllianceCode& genericAllianceCode,
                                           const DateTime& ticketDate,
                                           bool reduceTemporaryVectorsFallback)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  GenericAllianceCodeHistoricalKey key(genericAllianceCode);
  DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  if (!reduceTemporaryVectorsFallback)
  {
    return *ptr;
  }
  else
  {
    std::vector<AirlineAllianceContinentInfo*>* ret =
        new std::vector<AirlineAllianceContinentInfo*>;
    DAOCache::value_type::iterator i =
        find_if(ptr->begin(),
                ptr->end(),
                IsEffectiveHist<AirlineAllianceContinentInfo>(ticketDate, ticketDate));
    while (i != ptr->end())
    {
      ret->push_back(*i);
      i = find_if(
          ++i, ptr->end(), IsEffectiveH<AirlineAllianceContinentInfo>(ticketDate, ticketDate));
    }
    del.adopt(ret);
    return *ret;
  }
}

std::vector<AirlineAllianceContinentInfo*>*
AirlineAllianceContinentHistoricalDAO::create(GenericAllianceCodeHistoricalKey key)
{
  std::vector<AirlineAllianceContinentInfo*>* ret = new std::vector<AirlineAllianceContinentInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAirlineAllianceContinentHistorical fnc(dbAdapter->getAdapter());
    fnc.findAirlineAllianceContinentInfo(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in AirlineAllianceContinentHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
AirlineAllianceContinentHistoricalDAO::destroy(GenericAllianceCodeHistoricalKey key,
                                               std::vector<AirlineAllianceContinentInfo*>* recs)
{
  std::vector<AirlineAllianceContinentInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

size_t
AirlineAllianceContinentHistoricalDAO::invalidate(const ObjectKey& objectKey)
{
  return cache().clear();
}

std::string
AirlineAllianceContinentHistoricalDAO::_name("AirlineAllianceContinentHistorical");
std::string
AirlineAllianceContinentHistoricalDAO::_cacheClass("Common");
DAOHelper<AirlineAllianceContinentHistoricalDAO>
AirlineAllianceContinentHistoricalDAO::_helper(_name);
AirlineAllianceContinentHistoricalDAO* AirlineAllianceContinentHistoricalDAO::_instance = nullptr;
}
