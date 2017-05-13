//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/CurrencySelectionDAO.h"

#include "Common/Logger.h"
#include "Common/TseConsts.h"
#include "DBAccess/CurrencySelection.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetCurrencySelection.h"

namespace tse
{
log4cxx::LoggerPtr
CurrencySelectionDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.CurrencySelectionDAO"));
CurrencySelectionDAO&
CurrencySelectionDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<CurrencySelection*>&
getCurrencySelectionData(const NationCode& nation,
                         const DateTime& date,
                         DeleteList& deleteList,
                         const DateTime& ticketDate,
                         bool isHistorical)
{
  if (isHistorical)
  {
    CurrencySelectionHistoricalDAO& dao = CurrencySelectionHistoricalDAO::instance();
    return dao.get(deleteList, nation, date, ticketDate);
  }
  else
  {
    CurrencySelectionDAO& dao = CurrencySelectionDAO::instance();
    return dao.get(deleteList, nation, date, ticketDate);
  }
}

const std::vector<CurrencySelection*>&
CurrencySelectionDAO::get(DeleteList& del,
                          const NationCode& key,
                          const DateTime& date,
                          const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(NationKey(key));
  /*
  del.copy(ptr);
  std::vector<CurrencySelection*>* ret = new std::vector<CurrencySelection*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret),
  IsNotEffectiveG<CurrencySelection>(date, ticketDate));
  return *ret;
  */
  return *(applyFilter(del, ptr, IsNotEffectiveG<CurrencySelection>(date, ticketDate)));
}

void
CurrencySelectionDAO::load()
{
  StartupLoader<QueryGetAllCurSelBase, CurrencySelection, CurrencySelectionDAO>();
}

NationKey
CurrencySelectionDAO::createKey(CurrencySelection* info)
{
  return NationKey(info->nation());
}

std::vector<CurrencySelection*>*
CurrencySelectionDAO::create(NationKey key)
{
  std::vector<CurrencySelection*>* ret = new std::vector<CurrencySelection*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetOneCurSelBase cs(dbAdapter->getAdapter());
    cs.findCurrencySelection(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CurrencySelectionDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
CurrencySelectionDAO::destroy(NationKey key, std::vector<CurrencySelection*>* recs)
{
  destroyContainer(recs);
}

std::string
CurrencySelectionDAO::_name("CurrencySelection");
std::string
CurrencySelectionDAO::_cacheClass("Currency");
DAOHelper<CurrencySelectionDAO>
CurrencySelectionDAO::_helper(_name);
CurrencySelectionDAO* CurrencySelectionDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: CurrencySelectionHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
CurrencySelectionHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.CurrencySelectionHistoricalDAO"));
CurrencySelectionHistoricalDAO&
CurrencySelectionHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<CurrencySelection*>&
CurrencySelectionHistoricalDAO::get(DeleteList& del,
                                    const NationCode& key,
                                    const DateTime& date,
                                    const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<CurrencySelection*>* ret = new std::vector<CurrencySelection*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveHist<CurrencySelection>(date, ticketDate));
  return *ret;
}

struct CurrencySelectionHistoricalDAO::groupByKey
{
public:
  NationCode prevKey;

  DAOCache& cache;

  groupByKey()
    : prevKey(INVALID_NATIONCODE), cache(CurrencySelectionHistoricalDAO::instance().cache()), ptr(nullptr)
  {
  }

  std::vector<CurrencySelection*>* ptr;

  void operator()(CurrencySelection* info)
  {
    if (info->nation() != prevKey)
    {
      ptr = new std::vector<CurrencySelection*>;
      cache.put(info->nation(), ptr);
      prevKey = info->nation();
    }
    ptr->push_back(info);
  }
};

void
CurrencySelectionHistoricalDAO::load()
{
  if (!Global::allowHistorical())
    return;

  // Track calls for code coverage
  _codeCoverageLoadCallCount++;

  std::vector<CurrencySelection*> recs;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllCurSelBaseHistorical cs(dbAdapter->getAdapter());
    cs.findAllCurrencySelection(recs);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CurrencySelectionHistoricalDAO::load");
    deleteVectorOfPointers(recs);
    throw;
  }

  std::for_each(recs.begin(), recs.end(), groupByKey());
}

std::vector<CurrencySelection*>*
CurrencySelectionHistoricalDAO::create(NationCode key)
{
  std::vector<CurrencySelection*>* ret = new std::vector<CurrencySelection*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetOneCurSelBaseHistorical cs(dbAdapter->getAdapter());
    cs.findCurrencySelection(*ret, key);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CurrencySelectionHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
CurrencySelectionHistoricalDAO::destroy(NationCode key, std::vector<CurrencySelection*>* recs)
{
  std::vector<CurrencySelection*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
CurrencySelectionHistoricalDAO::_name("CurrencySelectionHistorical");
std::string
CurrencySelectionHistoricalDAO::_cacheClass("Currency");
DAOHelper<CurrencySelectionHistoricalDAO>
CurrencySelectionHistoricalDAO::_helper(_name);
CurrencySelectionHistoricalDAO* CurrencySelectionHistoricalDAO::_instance = nullptr;

} // namespace tse
