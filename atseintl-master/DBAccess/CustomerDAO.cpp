//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/CustomerDAO.h"

#include "Common/Logger.h"
#include "DBAccess/Customer.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetCustomer.h"

namespace tse
{
log4cxx::LoggerPtr
CustomerDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.CustomerDAO"));
CustomerDAO&
CustomerDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<Customer*>&
getCustomerData(const PseudoCityCode& key,
                DeleteList& deleteList,
                const DateTime& ticketDate,
                bool isHistorical)
{
  if (isHistorical)
  {
    CustomerHistoricalDAO& dao = CustomerHistoricalDAO::instance();
    return dao.get(deleteList, key, ticketDate);
  }
  else
  {
    CustomerDAO& dao = CustomerDAO::instance();
    return dao.get(deleteList, key, ticketDate);
  }
}

const std::vector<Customer*>&
CustomerDAO::get(DeleteList& del, const PseudoCityCode& key, const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(CustomerKey(key));
  del.copy(ptr);
  return *ptr;
}

std::vector<Customer*>*
CustomerDAO::create(CustomerKey key)
{
  std::vector<Customer*>* ret = new std::vector<Customer*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetCustomer cust(dbAdapter->getAdapter());
    cust.findCustomer(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CustomerDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
CustomerDAO::destroy(CustomerKey key, std::vector<Customer*>* recs)
{
  destroyContainer(recs);
}

std::string
CustomerDAO::_name("Customer");
std::string
CustomerDAO::_cacheClass("Common");
DAOHelper<CustomerDAO>
CustomerDAO::_helper(_name);
CustomerDAO* CustomerDAO::_instance = nullptr;

CustomerKey
CustomerDAO::createKey(const Customer* info)
{
  return CustomerKey(info->pseudoCity());
}

void
CustomerDAO::load()
{
  StartupLoaderNoDB<Customer, CustomerDAO>();
}

sfc::CompressedData*
CustomerDAO::compress(const std::vector<Customer*>* vect) const
{
  return compressVector(vect);
}

std::vector<Customer*>*
CustomerDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<Customer>(compressed);
}

// --------------------------------------------------
// Historical DAO: CustomerHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
CustomerHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.CustomerHistoricalDAO"));
CustomerHistoricalDAO&
CustomerHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<Customer*>&
CustomerHistoricalDAO::get(DeleteList& del, const PseudoCityCode& key, const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  // some of the objects aren't current, so we have to create a new vector to
  // return and copy all the current objects into it and return them.
  std::vector<Customer*>* ret = new std::vector<Customer*>(ptr->begin(), ptr->end());
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotCurrentCreateDateOnly<Customer>(ticketDate));

  return *ret;
}

std::vector<Customer*>*
CustomerHistoricalDAO::create(PseudoCityCode key)
{
  std::vector<Customer*>* ret = new std::vector<Customer*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetCustomer cust(dbAdapter->getAdapter());
    cust.findCustomer(*ret, key);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CustomerHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
CustomerHistoricalDAO::destroy(PseudoCityCode key, std::vector<Customer*>* recs)
{
  std::vector<Customer*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
CustomerHistoricalDAO::compress(const std::vector<Customer*>* vect) const
{
  return compressVector(vect);
}

std::vector<Customer*>*
CustomerHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<Customer>(compressed);
}

std::string
CustomerHistoricalDAO::_name("CustomerHistorical");
std::string
CustomerHistoricalDAO::_cacheClass("Common");
DAOHelper<CustomerHistoricalDAO>
CustomerHistoricalDAO::_helper(_name);
CustomerHistoricalDAO* CustomerHistoricalDAO::_instance = nullptr;

} // namespace tse
