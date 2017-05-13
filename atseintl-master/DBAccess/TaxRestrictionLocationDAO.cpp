//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/TaxRestrictionLocationDAO.h"

#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetTaxRestrictionLocation.h"
#include "DBAccess/TaxRestrictionLocationInfo.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
TaxRestrictionLocationDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.TaxRestrictionLocationDAO"));

TaxRestrictionLocationDAO&
TaxRestrictionLocationDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const TaxRestrictionLocationInfo*
getTaxRestrictionLocationData(const TaxRestrictionLocation& location,
                              DeleteList& deleteList,
                              const DateTime& ticketDate,
                              bool isHistorical)
{
  if (isHistorical)
  {
    TaxRestrictionLocationHistoricalDAO& dao = TaxRestrictionLocationHistoricalDAO::instance();
    const TaxRestrictionLocationInfo* curr = dao.get(deleteList, location, ticketDate);
    // ***
    // remove the next two lines when the general tables have been copied
    // to the historical database and replace "const TaxRestrictionLocationInfo* curr = " with
    // return
    if (curr)
      return curr;
    return getTaxRestrictionLocationData(location, deleteList, ticketDate, false);
    // ***
  }
  else
  {
    TaxRestrictionLocationDAO& dao = TaxRestrictionLocationDAO::instance();
    return dao.get(deleteList, location, ticketDate);
  }
}

const TaxRestrictionLocationInfo*
TaxRestrictionLocationDAO::get(DeleteList& del,
                               const TaxRestrictionLocation& location,
                               const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  TaxRestrictionLocationKey key(location);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  const TaxRestrictionLocationInfo* ret = nullptr;
  DAOCache::value_type::iterator i = find_if(
      ptr->begin(), ptr->end(), IsEffectiveG<TaxRestrictionLocationInfo>(ticketDate, ticketDate));
  if (i != ptr->end())
    ret = *i;
  return ret;
}

std::vector<const TaxRestrictionLocationInfo*>*
TaxRestrictionLocationDAO::create(TaxRestrictionLocationKey key)
{
  std::vector<const TaxRestrictionLocationInfo*>* ret =
      new std::vector<const TaxRestrictionLocationInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTaxRestrictionLocation zn(dbAdapter->getAdapter());
    zn.findTaxRestrictionLocation(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TaxRestrictionLocationDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
TaxRestrictionLocationDAO::destroy(TaxRestrictionLocationKey key,
                                   std::vector<const TaxRestrictionLocationInfo*>* recs)
{
  std::vector<const TaxRestrictionLocationInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i; // lint !e605
  delete recs;
}

std::string
TaxRestrictionLocationDAO::_name("TaxRestrictionLocation");
std::string
TaxRestrictionLocationDAO::_cacheClass("Taxes");

DAOHelper<TaxRestrictionLocationDAO>
TaxRestrictionLocationDAO::_helper(_name);

TaxRestrictionLocationDAO* TaxRestrictionLocationDAO::_instance = nullptr;

TaxRestrictionLocationKey
TaxRestrictionLocationDAO::createKey(const TaxRestrictionLocationInfo* info)
{
  return TaxRestrictionLocationKey(info->location());
}

void
TaxRestrictionLocationDAO::load()
{
  StartupLoaderNoDB<TaxRestrictionLocationInfo, TaxRestrictionLocationDAO>();
}

////////////////////////////////////
// Historical DAO: TaxRestrictionLocationHistoricalDAO
////////////////////////////////////

log4cxx::LoggerPtr
TaxRestrictionLocationHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.TaxRestrictionLocationHistoricalDAO"));

TaxRestrictionLocationHistoricalDAO&
TaxRestrictionLocationHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const TaxRestrictionLocationInfo*
TaxRestrictionLocationHistoricalDAO::get(DeleteList& del,
                                         const TaxRestrictionLocation& location,
                                         const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  TaxRestrictionLocationHistoricalKey key(location);
  DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  const TaxRestrictionLocationInfo* ret = nullptr;
  DAOCache::value_type::iterator i = find_if(
      ptr->begin(), ptr->end(), IsEffectiveH<TaxRestrictionLocationInfo>(ticketDate, ticketDate));
  if (i != ptr->end())
    ret = *i;
  return ret;
}

std::vector<const TaxRestrictionLocationInfo*>*
TaxRestrictionLocationHistoricalDAO::create(TaxRestrictionLocationHistoricalKey key)
{
  std::vector<const TaxRestrictionLocationInfo*>* ret =
      new std::vector<const TaxRestrictionLocationInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTaxRestrictionLocationHistorical zn(dbAdapter->getAdapter());
    zn.findTaxRestrictionLocation(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TaxRestrictionLocationHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

void
TaxRestrictionLocationHistoricalDAO::destroy(TaxRestrictionLocationHistoricalKey key,
                                             std::vector<const TaxRestrictionLocationInfo*>* recs)
{
  std::vector<const TaxRestrictionLocationInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i; // lint !e605
  delete recs;
}

std::string
TaxRestrictionLocationHistoricalDAO::_name("TaxRestrictionLocationHistorical");
std::string
TaxRestrictionLocationHistoricalDAO::_cacheClass("Taxes");

DAOHelper<TaxRestrictionLocationHistoricalDAO>
TaxRestrictionLocationHistoricalDAO::_helper(_name);

TaxRestrictionLocationHistoricalDAO* TaxRestrictionLocationHistoricalDAO::_instance = nullptr;

} // namespace tse
