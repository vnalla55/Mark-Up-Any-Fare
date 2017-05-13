//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DBAccess/ContractPreferenceDAO.h"

#include "Common/Logger.h"
#include "DBAccess/ContractPreference.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetContractPreferences.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
ContractPreferenceDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.ContractPreferenceDAO"));

ContractPreferenceDAO&
ContractPreferenceDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<ContractPreference*>&
getContractPreferencesData(const PseudoCityCode& pseudoCity,
                           const CarrierCode& carrier,
                           const RuleNumber& rule,
                           const DateTime& versionDate,
                           DeleteList& deleteList,
                           const DateTime& ticketDate,
                           bool isHistorical)
{
  if (isHistorical)
  {
    ContractPreferenceHistoricalDAO& dao = ContractPreferenceHistoricalDAO::instance();
    return dao.get(deleteList, pseudoCity, carrier, rule, versionDate, ticketDate);
  }
  else
  {
    ContractPreferenceDAO& dao = ContractPreferenceDAO::instance();
    return dao.get(deleteList, pseudoCity, carrier, rule, versionDate, ticketDate);
  }
}

const std::vector<ContractPreference*>&
ContractPreferenceDAO::get(DeleteList& del,
                           const PseudoCityCode& pseudoCity,
                           const CarrierCode& carrier,
                           const RuleNumber& rule,
                           const DateTime& date,
                           const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  ContractPreferenceKey key(pseudoCity, carrier, rule);
  DAOCache::pointer_type ptr = cache().get(key);
  /*
  del.copy(ptr);
  std::vector<ContractPreference*>* ret = new std::vector<ContractPreference*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),ptr->end(),back_inserter(*ret), IsNotEffectiveG<const
  ContractPreference>(date, ticketDate));

  return *ret;
  */
  return *(applyFilter(del, ptr, IsNotEffectiveG<const ContractPreference>(date, ticketDate)));
}

std::vector<ContractPreference*>*
ContractPreferenceDAO::create(ContractPreferenceKey key)
{
  std::vector<ContractPreference*>* ret = new std::vector<ContractPreference*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetContractPreferences cp(dbAdapter->getAdapter());
    cp.findContractPreferences(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in ContractPreferenceDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
ContractPreferenceDAO::destroy(ContractPreferenceKey key, std::vector<ContractPreference*>* recs)
{
  destroyContainer(recs);
}

sfc::CompressedData*
ContractPreferenceDAO::compress(const std::vector<ContractPreference*>* vect) const
{
  return compressVector(vect);
}

std::vector<ContractPreference*>*
ContractPreferenceDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<ContractPreference>(compressed);
}

std::string
ContractPreferenceDAO::_name("ContractPreference");
std::string
ContractPreferenceDAO::_cacheClass("Common");

DAOHelper<ContractPreferenceDAO>
ContractPreferenceDAO::_helper(_name);

ContractPreferenceDAO* ContractPreferenceDAO::_instance = nullptr;

ContractPreferenceKey
ContractPreferenceDAO::createKey(const ContractPreference* info)
{
  return ContractPreferenceKey(info->pseudoCity(), info->carrier(), info->rule());
}

void
ContractPreferenceDAO::load()
{
  StartupLoaderNoDB<ContractPreference, ContractPreferenceDAO>();
}

// --------------------------------------------------
// Historical DAO: ContractPreferenceHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
ContractPreferenceHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.ContractPreferenceHistoricalDAO"));
ContractPreferenceHistoricalDAO&
ContractPreferenceHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<ContractPreference*>&
ContractPreferenceHistoricalDAO::get(DeleteList& del,
                                     const PseudoCityCode& pseudoCity,
                                     const CarrierCode& carrier,
                                     const RuleNumber& rule,
                                     const DateTime& date,
                                     const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  ContractPreferenceKey key(pseudoCity, carrier, rule);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<ContractPreference*>* ret = new std::vector<ContractPreference*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveHist<const ContractPreference>(date, ticketDate));

  return *ret;
}

std::vector<ContractPreference*>*
ContractPreferenceHistoricalDAO::create(ContractPreferenceKey key)
{
  std::vector<ContractPreference*>* ret = new std::vector<ContractPreference*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetContractPreferencesHistorical cp(dbAdapter->getAdapter());
    cp.findContractPreferences(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in ContractPreferenceHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
ContractPreferenceHistoricalDAO::destroy(ContractPreferenceKey key,
                                         std::vector<ContractPreference*>* recs)
{
  destroyContainer(recs);
}

struct ContractPreferenceHistoricalDAO::groupByKey
{
public:
  ContractPreferenceKey prevKey;

  DAOCache& cache;

  groupByKey() : cache(ContractPreferenceHistoricalDAO::instance().cache()), ptr(nullptr) {}

  std::vector<ContractPreference*>* ptr;

  void operator()(ContractPreference* info)
  {
    ContractPreferenceKey key(info->pseudoCity(), info->carrier(), info->rule());
    if (!(key == prevKey))
    {
      ptr = new std::vector<ContractPreference*>;
      cache.put(key, ptr);
      prevKey = key;
    }
    ptr->push_back(info);
  }
};

void
ContractPreferenceHistoricalDAO::load()
{
  if (!Global::allowHistorical())
    return;

  // Track calls for code coverage
  _codeCoverageLoadCallCount++;

  std::vector<ContractPreference*> recs;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllContractPreferencesHistorical cpn(dbAdapter->getAdapter());
    cpn.findAllContractPreferences(recs);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in ContractPreferenceHistoricalDAO::load");
    deleteVectorOfPointers(recs);
    throw;
  }

  std::for_each(recs.begin(), recs.end(), groupByKey());
}

sfc::CompressedData*
ContractPreferenceHistoricalDAO::compress(const std::vector<ContractPreference*>* vect) const
{
  return compressVector(vect);
}

std::vector<ContractPreference*>*
ContractPreferenceHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<ContractPreference>(compressed);
}

std::string
ContractPreferenceHistoricalDAO::_name("ContractPreferenceHistorical");
std::string
ContractPreferenceHistoricalDAO::_cacheClass("Common");
DAOHelper<ContractPreferenceHistoricalDAO>
ContractPreferenceHistoricalDAO::_helper(_name);
ContractPreferenceHistoricalDAO* ContractPreferenceHistoricalDAO::_instance = nullptr;

} // namespace tse
