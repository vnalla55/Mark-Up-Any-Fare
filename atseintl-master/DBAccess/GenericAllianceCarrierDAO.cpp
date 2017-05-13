// ----------------------------------------------------------------
//
//   Copyright Sabre 2014
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
#include "DBAccess/GenericAllianceCarrierDAO.h"

#include "Common/Logger.h"
#include "DBAccess/AirlineAllianceCarrierInfo.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DAOUtils.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DBHistoryServer.h"
#include "DBAccess/Queries/QueryGetGenericAllianceCarrier.h"

namespace tse
{
Logger
GenericAllianceCarrierDAO::_logger("atseintl.DBAccess.GenericAllianceCarrierDAO");

GenericAllianceCarrierDAO&
GenericAllianceCarrierDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<AirlineAllianceCarrierInfo*>&
getGenericAllianceCarrierData(const GenericAllianceCode& genericAllianceCode,
                              DeleteList& deleteList,
                              const DateTime& ticketDate,
                              bool isHistorical)
{
  if (isHistorical)
  {
    GenericAllianceCarrierHistoricalDAO& dao = GenericAllianceCarrierHistoricalDAO::instance();
    const std::vector<AirlineAllianceCarrierInfo*>& ret(
        dao.get(deleteList, genericAllianceCode, ticketDate));

    return ret;
  }
  else
  {
    GenericAllianceCarrierDAO& dao = GenericAllianceCarrierDAO::instance();
    const std::vector<AirlineAllianceCarrierInfo*>& ret(
        dao.get(deleteList, genericAllianceCode, ticketDate));

    return ret;
  }
}

const std::vector<AirlineAllianceCarrierInfo*>&
GenericAllianceCarrierDAO::get(DeleteList& del,
                               const GenericAllianceCode& genericAllianceCode,
                               const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  GenericCarrierKey key(genericAllianceCode);
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
GenericAllianceCarrierDAO::create(GenericCarrierKey key)
{
  std::vector<AirlineAllianceCarrierInfo*>* ret = new std::vector<AirlineAllianceCarrierInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetGenericAllianceCarrier fnc(dbAdapter->getAdapter());
    fnc.findGenericAllianceCarrierInfo(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in GenericAllianceCarrierDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
GenericAllianceCarrierDAO::destroy(GenericCarrierKey key,
                                   std::vector<AirlineAllianceCarrierInfo*>* recs)
{
  destroyContainer(recs);
}

GenericCarrierKey
GenericAllianceCarrierDAO::createKey(AirlineAllianceCarrierInfo* info)
{
  return CarrierKey(info->genericAllianceCode());
}

void
GenericAllianceCarrierDAO::load()
{
  // not pre_loading

  StartupLoaderNoDB<AirlineAllianceCarrierInfo, GenericAllianceCarrierDAO>();
}

std::string
GenericAllianceCarrierDAO::_name("GenericAllianceCarrier");
std::string
GenericAllianceCarrierDAO::_cacheClass("Common");
DAOHelper<GenericAllianceCarrierDAO>
GenericAllianceCarrierDAO::_helper(_name);
GenericAllianceCarrierDAO* GenericAllianceCarrierDAO::_instance = nullptr;

///////////////////////////////////////////////////////
// Historical DAO Object
///////////////////////////////////////////////////////
Logger
GenericAllianceCarrierHistoricalDAO::_logger("atseintl.DBAccess.GenericAllianceCarrierHistDAO");
GenericAllianceCarrierHistoricalDAO&
GenericAllianceCarrierHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<AirlineAllianceCarrierInfo*>&
GenericAllianceCarrierHistoricalDAO::get(DeleteList& del,
                                         const GenericAllianceCode& genericAllianceCode,
                                         const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  GenericCarrierHistoricalKey key(genericAllianceCode);
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
GenericAllianceCarrierHistoricalDAO::create(GenericCarrierHistoricalKey key)
{
  std::vector<AirlineAllianceCarrierInfo*>* ret = new std::vector<AirlineAllianceCarrierInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetGenericAllianceCarrierHistorical fnc(dbAdapter->getAdapter());
    fnc.findGenericAllianceCarrierInfo(*ret, key._a, key._b, key._c);
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
GenericAllianceCarrierHistoricalDAO::destroy(GenericCarrierHistoricalKey key,
                                             std::vector<AirlineAllianceCarrierInfo*>* recs)
{
  std::vector<AirlineAllianceCarrierInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
GenericAllianceCarrierHistoricalDAO::_name("GenericAllianceCarrierHistorical");
std::string
GenericAllianceCarrierHistoricalDAO::_cacheClass("Common");
DAOHelper<GenericAllianceCarrierHistoricalDAO>
GenericAllianceCarrierHistoricalDAO::_helper(_name);
GenericAllianceCarrierHistoricalDAO* GenericAllianceCarrierHistoricalDAO::_instance = nullptr;
}
