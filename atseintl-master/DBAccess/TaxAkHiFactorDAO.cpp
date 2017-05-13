//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DBAccess/TaxAkHiFactorDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetTaxAkHiFactor.h"
#include "DBAccess/TaxAkHiFactor.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
TaxAkHiFactorDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.TaxAkHiFactorDAO"));

TaxAkHiFactorDAO&
TaxAkHiFactorDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<TaxAkHiFactor*>&
getTaxAkHiFactorData(const LocCode& key,
                     const DateTime& date,
                     DeleteList& deleteList,
                     const DateTime& ticketDate,
                     bool isHistorical)
{
  if (isHistorical)
  {
    TaxAkHiFactorHistoricalDAO& dao = TaxAkHiFactorHistoricalDAO::instance();
    return dao.get(deleteList, key, date, ticketDate);
  }
  else
  {
    TaxAkHiFactorDAO& dao = TaxAkHiFactorDAO::instance();
    return dao.get(deleteList, key, date, ticketDate);
  }
}

const std::vector<TaxAkHiFactor*>&
TaxAkHiFactorDAO::get(DeleteList& del,
                      const LocCode& key,
                      const DateTime& date,
                      const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(LocCodeKey(key));
  /*
  del.copy(ptr);
  std::vector<TaxAkHiFactor*>* ret = new std::vector<TaxAkHiFactor*>;
  del.adopt(ret);

  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), IsNotEffectiveG<TaxAkHiFactor>(date,
  ticketDate));

  return *ret;
  */
  return *(applyFilter(del, ptr, IsNotEffectiveG<TaxAkHiFactor>(date, ticketDate)));
}

LocCodeKey
TaxAkHiFactorDAO::createKey(TaxAkHiFactor* info)
{
  return LocCodeKey(info->city());
}

void
TaxAkHiFactorDAO::load()
{
  StartupLoader<QueryGetAllTaxAkHiFactor, TaxAkHiFactor, TaxAkHiFactorDAO>();
}

std::vector<TaxAkHiFactor*>*
TaxAkHiFactorDAO::create(LocCodeKey key)
{
  std::vector<TaxAkHiFactor*>* ret = new std::vector<TaxAkHiFactor*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetOneTaxAkHiFactor tahf(dbAdapter->getAdapter());
    tahf.findTaxAkHiFactor(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TaxAkHiFactorDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

void
TaxAkHiFactorDAO::destroy(LocCodeKey key, std::vector<TaxAkHiFactor*>* recs)
{
  destroyContainer(recs);
}

std::string
TaxAkHiFactorDAO::_name("TaxAkHiFactor");
std::string
TaxAkHiFactorDAO::_cacheClass("Taxes");

DAOHelper<TaxAkHiFactorDAO>
TaxAkHiFactorDAO::_helper(_name);

TaxAkHiFactorDAO* TaxAkHiFactorDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: TaxAkHiFactorHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
TaxAkHiFactorHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.TaxAkHiFactorHistoricalDAO"));

TaxAkHiFactorHistoricalDAO&
TaxAkHiFactorHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<TaxAkHiFactor*>&
TaxAkHiFactorHistoricalDAO::get(DeleteList& del,
                                const LocCode& key,
                                const DateTime& date,
                                const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  TaxAkHiFactorHistoricalKey cacheKey(key);
  DAOUtils::getDateRange(ticketDate, cacheKey._b, cacheKey._c, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(cacheKey);
  del.copy(ptr);
  std::vector<TaxAkHiFactor*>* ret = new std::vector<TaxAkHiFactor*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveHist<TaxAkHiFactor>(date, ticketDate));
  return *ret;
}

std::vector<TaxAkHiFactor*>*
TaxAkHiFactorHistoricalDAO::create(TaxAkHiFactorHistoricalKey key)
{
  std::vector<TaxAkHiFactor*>* ret = new std::vector<TaxAkHiFactor*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetOneTaxAkHiFactorHistorical tahfh(dbAdapter->getAdapter());
    tahfh.findTaxAkHiFactorHistorical(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TaxAkHiFactorHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
TaxAkHiFactorHistoricalDAO::destroy(const TaxAkHiFactorHistoricalKey key,
                                    std::vector<TaxAkHiFactor*>* recs)
{
  std::vector<TaxAkHiFactor*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
TaxAkHiFactorHistoricalDAO::_name("TaxAkHiFactorHistorical");
std::string
TaxAkHiFactorHistoricalDAO::_cacheClass("Taxes");

DAOHelper<TaxAkHiFactorHistoricalDAO>
TaxAkHiFactorHistoricalDAO::_helper(_name);

TaxAkHiFactorHistoricalDAO* TaxAkHiFactorHistoricalDAO::_instance = nullptr;
} // namespace tse
