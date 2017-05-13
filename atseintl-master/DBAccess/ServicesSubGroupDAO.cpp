// ----------------------------------------------------------------
//
//   Copyright Sabre 2012
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

#include "DBAccess/ServicesSubGroupDAO.h"

#include "DBAccess/Queries/QueryGetServicesSubGroup.h"
#include "DBAccess/ServicesSubGroup.h"

namespace tse
{
log4cxx::LoggerPtr
ServicesSubGroupDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.ServicesSubGroupDAO"));

std::string
ServicesSubGroupDAO::_name("ServicesSubGroup");
std::string
ServicesSubGroupDAO::_cacheClass("Rules");
DAOHelper<ServicesSubGroupDAO>
ServicesSubGroupDAO::_helper(_name);
ServicesSubGroupDAO* ServicesSubGroupDAO::_instance = nullptr;

ServicesSubGroupDAO&
ServicesSubGroupDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const ServicesSubGroup*
getServicesSubGroupData(const ServiceGroup& serviceGroup,
                        const ServiceGroup& serviceSubGroup,
                        DeleteList& deleteList,
                        const DateTime& ticketDate,
                        bool isHistorical)
{
  ServicesSubGroupDAO& dao = ServicesSubGroupDAO::instance();
  return dao.get(deleteList, serviceGroup, serviceSubGroup, ticketDate);
}

const ServicesSubGroup*
ServicesSubGroupDAO::get(DeleteList& del,
                         const ServiceGroup& serviceGroup,
                         const ServiceGroup& serviceSubGroup,
                         const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  ServicesSubGroupKey key(serviceGroup, serviceSubGroup);
  DAOCache::pointer_type ptr = cache().get(key);

  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  DAOCache::value_type::iterator iter = ptr->begin();
  // select the youngest element that is older than ticketData
  for (; iter != ptr->end(); ++iter)
  {
    if (static_cast<ServicesSubGroup*>(*iter)->createDate() < ticketDate)
    {
      return *iter;
    }
  }

  return nullptr;
}

std::vector<ServicesSubGroup*>*
ServicesSubGroupDAO::create(ServicesSubGroupKey key)
{
  std::vector<ServicesSubGroup*>* ret = new std::vector<ServicesSubGroup*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());

  try
  {
    QueryGetServicesSubGroup fnc(dbAdapter->getAdapter());
    fnc.findServicesSubGroup(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in ServicesSubGroupDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
ServicesSubGroupDAO::destroy(ServicesSubGroupKey key, std::vector<ServicesSubGroup*>* recs)
{
  destroyContainer(recs);
}

ServicesSubGroupKey
ServicesSubGroupDAO::createKey(ServicesSubGroup* servicesSubGroup)
{
  return ServicesSubGroupKey(servicesSubGroup->serviceGroup(), servicesSubGroup->serviceSubGroup());
}

void
ServicesSubGroupDAO::load()
{
  // not pre_loading
  StartupLoaderNoDB<QueryGetServicesSubGroup, ServicesSubGroupDAO>();
}
} // tse
