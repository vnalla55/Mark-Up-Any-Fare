//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/NvbNvaDAO.h"

#include "DBAccess/NvbNvaInfo.h"
#include "DBAccess/Queries/QueryGetNvbNva.h"

namespace tse
{
log4cxx::LoggerPtr
NvbNvaDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.NvbNvaDAO"));

std::string
NvbNvaDAO::_name("NvbNva");
std::string
NvbNvaDAO::_cacheClass("Rules");
DAOHelper<NvbNvaDAO>
NvbNvaDAO::_helper(_name);
NvbNvaDAO* NvbNvaDAO::_instance = nullptr;

NvbNvaDAO&
NvbNvaDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<NvbNvaInfo*>&
getNvbNvaData(const VendorCode& vendor,
              const CarrierCode& carrier,
              const TariffNumber& ruleTariff,
              const RuleNumber& rule,
              DeleteList& deleteList,
              const DateTime& ticketDate,
              bool isHistorical)
{
  if (isHistorical)
  {
    NvbNvaHistoricalDAO& dao = NvbNvaHistoricalDAO::instance();
    return dao.get(deleteList, vendor, carrier, ruleTariff, rule, ticketDate);
  }
  else
  {
    NvbNvaDAO& dao = NvbNvaDAO::instance();
    return dao.get(deleteList, vendor, carrier, ruleTariff, rule, ticketDate);
  }
}

const std::vector<NvbNvaInfo*>&
NvbNvaDAO::get(DeleteList& del,
               const VendorCode& vendor,
               const CarrierCode& carrier,
               const TariffNumber& ruleTariff,
               const RuleNumber& rule,
               const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  // get data from cache
  NvbNvaKey key(vendor, carrier, ruleTariff, rule);
  DAOCache::pointer_type ptr = cache().get(key);

  // copy only the current results to result vector
  return *(applyFilter(del, ptr, IsNotCurrentG<NvbNvaInfo>(ticketDate)));
}

std::vector<NvbNvaInfo*>*
NvbNvaDAO::create(NvbNvaKey key)
{
  std::vector<NvbNvaInfo*>* ret = new std::vector<NvbNvaInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetNvbNva fnc(dbAdapter->getAdapter());
    fnc.findNvbNvaInfo(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in NvbNvaDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
NvbNvaDAO::destroy(NvbNvaKey key, std::vector<NvbNvaInfo*>* recs)
{
  destroyContainer(recs);
}

NvbNvaKey
NvbNvaDAO::createKey(NvbNvaInfo* info)
{
  return NvbNvaKey(info->vendor(), info->carrier(), info->ruleTariff(), info->rule());
}

void
NvbNvaDAO::load()
{
  // StartupLoader<QueryGetAllNvbNva,NvbNvaInfo,NvbNvaDAO>();
  // TODO substitude below line with the one above when we are ready to deploy LDC functionality
  StartupLoaderNoDB<NvbNvaInfo, NvbNvaDAO>();
}

///////////////////////////////////////////////////////
// Historical DAO Object
///////////////////////////////////////////////////////
log4cxx::LoggerPtr
NvbNvaHistoricalDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.NvbNvaHistoricalDAO"));

std::string
NvbNvaHistoricalDAO::_name("NvbNvaHistorical");
std::string
NvbNvaHistoricalDAO::_cacheClass("Rules");
DAOHelper<NvbNvaHistoricalDAO>
NvbNvaHistoricalDAO::_helper(_name);
NvbNvaHistoricalDAO* NvbNvaHistoricalDAO::_instance = nullptr;

NvbNvaHistoricalDAO&
NvbNvaHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<NvbNvaInfo*>&
NvbNvaHistoricalDAO::get(DeleteList& del,
                         const VendorCode& vendor,
                         const CarrierCode& carrier,
                         const TariffNumber& ruleTariff,
                         const RuleNumber& rule,
                         const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  // get data from cache
  NvbNvaHistoricalKey key(vendor, carrier, ruleTariff, rule);
  DAOUtils::getDateRange(ticketDate, key._e, key._f, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);

  // copy only the current results to result vector
  del.copy(ptr);
  std::vector<NvbNvaInfo*>* ret = new std::vector<NvbNvaInfo*>;
  del.adopt(ret);
  remove_copy_if(
      ptr->begin(), ptr->end(), back_inserter(*ret), IsNotCurrentH<NvbNvaInfo>(ticketDate));

  // return results
  return *ret;
}

std::vector<NvbNvaInfo*>*
NvbNvaHistoricalDAO::create(NvbNvaHistoricalKey key)
{
  std::vector<NvbNvaInfo*>* ret = new std::vector<NvbNvaInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetNvbNvaHistorical fnc(dbAdapter->getAdapter());
    fnc.findNvbNvaInfo(*ret, key._a, key._b, key._c, key._d, key._e, key._f);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in NvbNvaHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
NvbNvaHistoricalDAO::destroy(NvbNvaHistoricalKey key, std::vector<NvbNvaInfo*>* recs)
{
  std::vector<NvbNvaInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

} // namespace tse
