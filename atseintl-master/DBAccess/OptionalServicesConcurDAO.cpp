//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/OptionalServicesConcurDAO.h"

#include "DBAccess/OptionalServicesConcur.h"
#include "DBAccess/Queries/QueryGetOptionalServicesConcur.h"

namespace tse
{
log4cxx::LoggerPtr
OptionalServicesConcurDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.OptionalServicesConcurDAO"));

std::string
OptionalServicesConcurDAO::_name("OptionalServicesConcur");
std::string
OptionalServicesConcurDAO::_cacheClass("Rules");
DAOHelper<OptionalServicesConcurDAO>
OptionalServicesConcurDAO::_helper(_name);
OptionalServicesConcurDAO* OptionalServicesConcurDAO::_instance = nullptr;

OptionalServicesConcurDAO&
OptionalServicesConcurDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<OptionalServicesConcur*>&
getOptionalServicesConcurData(const VendorCode& vendor,
                              const CarrierCode& carrier,
                              const ServiceTypeCode& serviceTypeCode,
                              const DateTime& date,
                              DeleteList& deleteList,
                              const DateTime& ticketDate,
                              bool isHistorical)
{
  if (isHistorical)
  {
    OptionalServicesConcurHistoricalDAO& dao = OptionalServicesConcurHistoricalDAO::instance();
    return dao.get(deleteList, vendor, carrier, serviceTypeCode, date, ticketDate);
  }
  else
  {
    OptionalServicesConcurDAO& dao = OptionalServicesConcurDAO::instance();
    return dao.get(deleteList, vendor, carrier, serviceTypeCode, date, ticketDate);
  }
}

const std::vector<OptionalServicesConcur*>&
OptionalServicesConcurDAO::get(DeleteList& del,
                               const VendorCode& vendor,
                               const CarrierCode& carrier,
                               const ServiceTypeCode& serviceTypeCode,
                               const DateTime& date,
                               const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  OptionalServicesConcurKey key(vendor, carrier, serviceTypeCode);
  DAOCache::pointer_type ptr = cache().get(key);
  return *applyFilter(del, ptr, IsNotEffectiveG<OptionalServicesConcur>(date, ticketDate));
}

std::vector<OptionalServicesConcur*>*
OptionalServicesConcurDAO::create(OptionalServicesConcurKey key)
{
  std::vector<OptionalServicesConcur*>* ret = new std::vector<OptionalServicesConcur*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetOptionalServicesConcur fnc(dbAdapter->getAdapter());
    fnc.findOptionalServicesConcur(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in OptionalServicesConcurDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
OptionalServicesConcurDAO::destroy(OptionalServicesConcurKey key,
                                   std::vector<OptionalServicesConcur*>* recs)
{
  destroyContainer(recs);
}

OptionalServicesConcurKey
OptionalServicesConcurDAO::createKey(OptionalServicesConcur* concur)
{
  return OptionalServicesConcurKey(concur->vendor(), concur->carrier(), concur->serviceTypeCode());
}

size_t
OptionalServicesConcurDAO::invalidate(const ObjectKey& objectKey)
{
  size_t result(0);
  OptionalServicesConcurKey invKey;
  if (translateKey(objectKey, invKey))
  {
    // Flush all possible OPTIONALSERVICESCONCUR objects regardless of the SEQNO: ._e value
    std::shared_ptr<std::vector<OptionalServicesConcurKey>> cacheKeys = cache().keys();
    std::vector<OptionalServicesConcurKey>* pCacheKeys = cacheKeys.get();
    if (pCacheKeys != nullptr)
    {
      int nKeys = pCacheKeys->size();
      for (int i = 0; (i < nKeys); i++)
      {
        OptionalServicesConcurKey cacheKey = (*pCacheKeys)[i];
        if ((invKey._a == cacheKey._a) && (invKey._b == cacheKey._b))
        {
          result += cache().invalidate(cacheKey);
        }
      }
    }
  }
  return result;
}

void
OptionalServicesConcurDAO::load()
{
  // not pre_loading
  StartupLoaderNoDB<OptionalServicesConcur, OptionalServicesConcurDAO>();
}

///////////////////////////////////////////////////////
// Historical DAO Object
///////////////////////////////////////////////////////
log4cxx::LoggerPtr
OptionalServicesConcurHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.OptionalServicesConcurHistoricalDAO"));

std::string
OptionalServicesConcurHistoricalDAO::_name("OptionalServicesConcurHistorical");
std::string
OptionalServicesConcurHistoricalDAO::_cacheClass("Rules");
DAOHelper<OptionalServicesConcurHistoricalDAO>
OptionalServicesConcurHistoricalDAO::_helper(_name);
OptionalServicesConcurHistoricalDAO* OptionalServicesConcurHistoricalDAO::_instance = nullptr;

OptionalServicesConcurHistoricalDAO&
OptionalServicesConcurHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<OptionalServicesConcur*>&
OptionalServicesConcurHistoricalDAO::get(DeleteList& del,
                                         const VendorCode& vendor,
                                         const CarrierCode& carrier,
                                         const ServiceTypeCode& serviceTypeCode,
                                         const DateTime& date,
                                         const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  OptionalServicesConcurHistoricalKey key(vendor, carrier, serviceTypeCode);
  DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<OptionalServicesConcur*>* ret = new std::vector<OptionalServicesConcur*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveH<OptionalServicesConcur>(date, ticketDate));

  return *ret;
}

std::vector<OptionalServicesConcur*>*
OptionalServicesConcurHistoricalDAO::create(OptionalServicesConcurHistoricalKey key)
{
  std::vector<OptionalServicesConcur*>* ret = new std::vector<OptionalServicesConcur*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetOptionalServicesConcurHistorical fnc(dbAdapter->getAdapter());
    fnc.findOptionalServicesConcur(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in OptionalServicesConcurHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
OptionalServicesConcurHistoricalDAO::destroy(OptionalServicesConcurHistoricalKey key,
                                             std::vector<OptionalServicesConcur*>* recs)
{
  std::vector<OptionalServicesConcur*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

size_t
OptionalServicesConcurHistoricalDAO::invalidate(const ObjectKey& objectKey)
{
  OptionalServicesConcurHistoricalKey invKey;
  size_t result(0);
  if (translateKey(objectKey, invKey))
  {
    // Flush all possible OptionalServicesConcur objects
    std::shared_ptr<std::vector<OptionalServicesConcurHistoricalKey>> cacheKeys = cache().keys();
    std::vector<OptionalServicesConcurHistoricalKey>* pCacheKeys = cacheKeys.get();
    if (pCacheKeys != nullptr)
    {
      int nKeys = pCacheKeys->size();
      for (int i = 0; (i < nKeys); i++)
      {
        OptionalServicesConcurHistoricalKey cacheKey = (*pCacheKeys)[i];
        if ((invKey._a == cacheKey._a) && (invKey._b == cacheKey._b))
        {
          result += cache().invalidate(cacheKey);
        }
      }
    }
  }
  return result;
}

} // tse
