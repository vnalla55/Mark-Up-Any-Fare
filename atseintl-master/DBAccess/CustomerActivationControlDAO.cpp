//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/CustomerActivationControlDAO.h"

#include "DBAccess/CustomerActivationControl.h"
#include "DBAccess/Queries/QueryGetCustomerActivationControl.h"

namespace tse
{
log4cxx::LoggerPtr
CustomerActivationControlDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.CustomerActivationControlDAO"));

std::string
CustomerActivationControlDAO::_name("CustomerActivationControl");
std::string
CustomerActivationControlDAO::_cacheClass("Common");
DAOHelper<CustomerActivationControlDAO>
CustomerActivationControlDAO::_helper(_name);
CustomerActivationControlDAO* CustomerActivationControlDAO::_instance = nullptr;

CustomerActivationControlDAO&
CustomerActivationControlDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<tse::CustomerActivationControl*>&
getCustomerActivationControlData(const std::string& projectCode,
                                 const DateTime& date,
                                 DeleteList& deleteList,
                                 const DateTime& ticketDate,
                                 bool isHistorical)
{

  if (isHistorical)
  {
    CustomerActivationControlHistoricalDAO& dao =
        CustomerActivationControlHistoricalDAO::instance();
    return dao.get(deleteList, projectCode, date, ticketDate);
  }
  else
  {
    CustomerActivationControlDAO& dao = CustomerActivationControlDAO::instance();
    return dao.get(deleteList, projectCode, date, ticketDate);
  }
}

const std::vector<tse::CustomerActivationControl*>&
CustomerActivationControlDAO::get(DeleteList& del,
                                  const std::string& projectCode,
                                  const DateTime& date,
                                  const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  CustomerActivationControlKey key(projectCode);
  DAOCache::pointer_type ptr = cache().get(key);

  del.copy(ptr);
  std::vector<CustomerActivationControl*>* ret = new std::vector<CustomerActivationControl*>;
  del.adopt(ret);

  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveG<CustomerActivationControl>(date, ticketDate));

  return *ret;
}

std::vector<CustomerActivationControl*>*
CustomerActivationControlDAO::create(CustomerActivationControlKey key)
{
  std::vector<CustomerActivationControl*>* ret = new std::vector<CustomerActivationControl*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetCustomerActivationControl fnc(dbAdapter->getAdapter());
    fnc.findCustomerActivationControl(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CustomerActivationControlDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
CustomerActivationControlDAO::destroy(CustomerActivationControlKey key,
                                      std::vector<CustomerActivationControl*>* recs)
{
  destroyContainer(recs);
}

CustomerActivationControlKey
CustomerActivationControlDAO::createKey(CustomerActivationControl* info)
{
  return CustomerActivationControlKey(info->projCode());
}

void
CustomerActivationControlDAO::load()
{
  // not pre_loading
  StartupLoaderNoDB<CustomerActivationControl, CustomerActivationControlDAO>();
}

///////////////////////////////////////////////////////
// Historical DAO Object
///////////////////////////////////////////////////////
log4cxx::LoggerPtr
CustomerActivationControlHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.CustomerActivationControlHistoricalDAO"));

std::string
CustomerActivationControlHistoricalDAO::_name("CustomerActivationControlHistorical");
std::string
CustomerActivationControlHistoricalDAO::_cacheClass("Common");
DAOHelper<CustomerActivationControlHistoricalDAO>
CustomerActivationControlHistoricalDAO::_helper(_name);
CustomerActivationControlHistoricalDAO* CustomerActivationControlHistoricalDAO::_instance = nullptr;

CustomerActivationControlHistoricalDAO&
CustomerActivationControlHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<tse::CustomerActivationControl*>&
CustomerActivationControlHistoricalDAO::get(DeleteList& del,
                                            const std::string& projectCode,
                                            const DateTime& date,
                                            const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  CustomerActivationControlHistoricalKey key(projectCode);
  DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<CustomerActivationControl*>* ret = new std::vector<CustomerActivationControl*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveH<CustomerActivationControl>(date, ticketDate));

  return *ret;
}

std::vector<CustomerActivationControl*>*
CustomerActivationControlHistoricalDAO::create(CustomerActivationControlHistoricalKey key)
{
  std::vector<CustomerActivationControl*>* ret = new std::vector<CustomerActivationControl*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetCustomerActivationControlHistorical fnc(dbAdapter->getAdapter());
    fnc.findCustomerActivationControl(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CustomerActivationControlHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
CustomerActivationControlHistoricalDAO::destroy(CustomerActivationControlHistoricalKey key,
                                                std::vector<CustomerActivationControl*>* recs)
{
  std::vector<CustomerActivationControl*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}
} // tse
