#include "DBAccess/ServicesDescriptionDAO.h"

#include "DBAccess/Queries/QueryGetServicesDescription.h"
#include "DBAccess/ServicesDescription.h"

namespace tse
{
log4cxx::LoggerPtr
ServicesDescriptionDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.ServicesDescriptionDAO"));

std::string
ServicesDescriptionDAO::_name("ServicesDescription");
std::string
ServicesDescriptionDAO::_cacheClass("Rules");
DAOHelper<ServicesDescriptionDAO>
ServicesDescriptionDAO::_helper(_name);
ServicesDescriptionDAO* ServicesDescriptionDAO::_instance = nullptr;

ServicesDescriptionDAO&
ServicesDescriptionDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const ServicesDescription*
getServicesDescriptionData(const ServiceGroupDescription& value,
                           DeleteList& deleteList,
                           const DateTime& ticketDate,
                           bool isHistorical)
{
  if (isHistorical)
  {
    ServicesDescriptionHistoricalDAO& dao = ServicesDescriptionHistoricalDAO::instance();
    return dao.get(deleteList, value, ticketDate);
  }
  else
  {
    ServicesDescriptionDAO& dao = ServicesDescriptionDAO::instance();
    return dao.get(deleteList, value, ticketDate);
  }
}

const ServicesDescription*
ServicesDescriptionDAO::get(DeleteList& del,
                            const ServiceGroupDescription& carrier,
                            const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  ServicesDescriptionKey key(carrier);
  DAOCache::pointer_type ptr = cache().get(key);

  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsEffectiveG<ServicesDescription> isEffective(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();

  for (; iter != ptr->end(); ++iter)
  {
    if (isEffective(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<ServicesDescription*>*
ServicesDescriptionDAO::create(ServicesDescriptionKey key)
{
  std::vector<ServicesDescription*>* ret = new std::vector<ServicesDescription*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());

  try
  {
    QueryGetServicesDescription fnc(dbAdapter->getAdapter());
    fnc.findServicesDescription(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in ServicesDescriptionDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
ServicesDescriptionDAO::destroy(ServicesDescriptionKey key, std::vector<ServicesDescription*>* recs)
{
  destroyContainer(recs);
}

ServicesDescriptionKey
ServicesDescriptionDAO::createKey(ServicesDescription* info)
{
  return ServicesDescriptionKey(info->value());
}

void
ServicesDescriptionDAO::load()
{
  // not pre_loading
  StartupLoaderNoDB<QueryGetServicesDescription, ServicesDescriptionDAO>();
}

///////////////////////////////////////////////////////
// Historical DAO Object
///////////////////////////////////////////////////////
log4cxx::LoggerPtr
ServicesDescriptionHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.ServicesDescriptionHistoricalDAO"));
std::string
ServicesDescriptionHistoricalDAO::_name("ServicesDescriptionHistorical");
std::string
ServicesDescriptionHistoricalDAO::_cacheClass("Rules");
DAOHelper<ServicesDescriptionHistoricalDAO>
ServicesDescriptionHistoricalDAO::_helper(_name);
ServicesDescriptionHistoricalDAO* ServicesDescriptionHistoricalDAO::_instance = nullptr;

ServicesDescriptionHistoricalDAO&
ServicesDescriptionHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const ServicesDescription*
ServicesDescriptionHistoricalDAO::get(DeleteList& del,
                                      const ServiceGroupDescription& carrier,
                                      const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  ServicesDescriptionHistoricalKey key(carrier);
  DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  if (ptr->empty())
    return nullptr;

  del.copy(ptr);
  IsEffectiveH<ServicesDescription> isEffective(ticketDate, ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();

  for (; iter != ptr->end(); ++iter)
  {
    if (isEffective(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<ServicesDescription*>*
ServicesDescriptionHistoricalDAO::create(ServicesDescriptionHistoricalKey key)
{
  std::vector<ServicesDescription*>* ret = new std::vector<ServicesDescription*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());

  try
  {
    QueryGetServicesDescriptionHistorical fnc(dbAdapter->getAdapter());
    fnc.findServicesDescription(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in ServicesDescriptionHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
ServicesDescriptionHistoricalDAO::destroy(ServicesDescriptionHistoricalKey key,
                                          std::vector<ServicesDescription*>* recs)
{
  std::vector<ServicesDescription*>::iterator i;

  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;

  delete recs;
}
} // tse
