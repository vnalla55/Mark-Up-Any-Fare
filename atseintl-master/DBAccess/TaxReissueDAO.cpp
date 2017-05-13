//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/TaxReissueDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetTaxReissue.h"
#include "DBAccess/TaxReissue.h"

namespace tse
{
log4cxx::LoggerPtr
TaxReissueDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.TaxReissueDAO"));
TaxReissueDAO&
TaxReissueDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<TaxReissue*>&
getTaxReissueData(const TaxCode& taxCode,
                  const DateTime& date,
                  DeleteList& deleteList,
                  const DateTime& ticketDate,
                  bool isHistorical)
{
  if (isHistorical)
  {
    TaxReissueHistoricalDAO& dao = TaxReissueHistoricalDAO::instance();
    return dao.get(deleteList, taxCode, date, ticketDate);
  }
  else
  {
    TaxReissueDAO& dao = TaxReissueDAO::instance();
    return dao.get(deleteList, taxCode, date, ticketDate);
  }
}

const std::vector<TaxReissue*>&
TaxReissueDAO::get(DeleteList& del,
                   const TaxCode& key,
                   const DateTime& date,
                   const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(key);
  /*
  del.copy(ptr);
  std::vector<TaxReissue*>* ret = new std::vector<TaxReissue*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), IsNotEffectiveG<TaxReissue>(date,
  ticketDate));
  return *ret;
  */
  return *(applyFilter(del, ptr, IsNotEffectiveG<TaxReissue>(date, ticketDate)));
}

std::vector<TaxReissue*>*
TaxReissueDAO::create(TaxCode key)
{
  std::vector<TaxReissue*>* ret = new std::vector<TaxReissue*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTaxReissue tc(dbAdapter->getAdapter());
    tc.findTaxReissue(*ret, key);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TaxReissueDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

sfc::CompressedData*
TaxReissueDAO::compress(const std::vector<TaxReissue*>* vect) const
{
  return compressVector(vect);
}

std::vector<TaxReissue*>*
TaxReissueDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<TaxReissue>(compressed);
}

void
TaxReissueDAO::destroy(TaxCode key, std::vector<TaxReissue*>* recs)
{
  std::vector<TaxReissue*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
TaxReissueDAO::_name("TaxReissue");
std::string
TaxReissueDAO::_cacheClass("Taxes");
DAOHelper<TaxReissueDAO>
TaxReissueDAO::_helper(_name);
TaxReissueDAO* TaxReissueDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: TaxReissueHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
TaxReissueHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.TaxReissueHistoricalDAO"));
TaxReissueHistoricalDAO&
TaxReissueHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<TaxReissue*>&
TaxReissueHistoricalDAO::get(DeleteList& del,
                             const TaxCode& key,
                             const DateTime& date,
                             const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<TaxReissue*>* ret = new std::vector<TaxReissue*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveHist<TaxReissue>(date, ticketDate));
  return *ret;
}

struct TaxReissueHistoricalDAO::groupByKey
{
public:
  TaxCode prevKey;

  DAOCache& cache;

  groupByKey() : cache(TaxReissueHistoricalDAO::instance().cache()), ptr(nullptr) {}

  std::vector<TaxReissue*>* ptr;

  void operator()(TaxReissue* info)
  {
    TaxCode key(info->taxCode());
    if (!(key == prevKey))
    {
      ptr = new std::vector<TaxReissue*>;
      cache.put(key, ptr);
      prevKey = key;
    }

    ptr->push_back(info);
  }
}; // struct TaxReissueHistoricalDAO::groupByKey

void
TaxReissueHistoricalDAO::load()
{
  if (!Global::allowHistorical())
    return;

  // Track calls for code coverage
  _codeCoverageLoadCallCount++;

  std::vector<TaxReissue*> recs;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllTaxReissueHistorical tcr(dbAdapter->getAdapter());
    tcr.findAllTaxReissues(recs);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TaxReissueHistoricalDAO::load");
    deleteVectorOfPointers(recs);
    throw;
  }

  std::for_each(recs.begin(), recs.end(), groupByKey());
}

std::vector<TaxReissue*>*
TaxReissueHistoricalDAO::create(TaxCode key)
{
  std::vector<TaxReissue*>* ret = new std::vector<TaxReissue*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTaxReissueHistorical tc(dbAdapter->getAdapter());
    tc.findTaxReissue(*ret, key);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TaxReissueHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

sfc::CompressedData*
TaxReissueHistoricalDAO::compress(const std::vector<TaxReissue*>* vect) const
{
  return compressVector(vect);
}

std::vector<TaxReissue*>*
TaxReissueHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<TaxReissue>(compressed);
}

void
TaxReissueHistoricalDAO::destroy(TaxCode key, std::vector<TaxReissue*>* recs)
{
  std::vector<TaxReissue*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
TaxReissueHistoricalDAO::_name("TaxReissueHistorical");
std::string
TaxReissueHistoricalDAO::_cacheClass("Taxes");
DAOHelper<TaxReissueHistoricalDAO>
TaxReissueHistoricalDAO::_helper(_name);
TaxReissueHistoricalDAO* TaxReissueHistoricalDAO::_instance = nullptr;

} // namespace tse
