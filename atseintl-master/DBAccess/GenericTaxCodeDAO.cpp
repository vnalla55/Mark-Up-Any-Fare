//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DBAccess/GenericTaxCodeDAO.h"

#include "Common/Logger.h"
#include "Common/TseConsts.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/GenericTaxCode.h"
#include "DBAccess/Queries/QueryGetAllGenericTaxCode.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
GenericTaxCodeDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.GenericTaxCodeDAO"));

GenericTaxCodeDAO&
GenericTaxCodeDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<GenericTaxCode*>&
GenericTaxCodeDAO::get(DeleteList& del,
                       const TaxCode& key,
                       const DateTime& date,
                       const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  IsEffectiveG<GenericTaxCode> isEffective(date, ticketDate);

  DAOCache::pointer_type ptr = cache().get(IntKey(0));
  del.copy(ptr);
  std::vector<GenericTaxCode*>* ret = new std::vector<GenericTaxCode*>;
  del.adopt(ret);
  DAOCache::value_type::iterator i = find_if(ptr->begin(), ptr->end(), isEffective);
  while (i != ptr->end())
  {
    if ((*i)->taxCode() == key)
      ret->push_back(*i);
    i = find_if(++i, ptr->end(), isEffective);
  }
  return *ret;
}

const std::vector<GenericTaxCode*>&
GenericTaxCodeDAO::getAll(DeleteList& del, const DateTime& date)
{
  // Track calls for code coverage
  _codeCoverageGetAllCallCount++;

  DAOCache::pointer_type ptr = cache().get(IntKey(0));
  return *(applyFilter(del, ptr, IsNotEffectiveG<GenericTaxCode>(date, date, date)));
}

IntKey
GenericTaxCodeDAO::createKey(GenericTaxCode* info)
{
  return IntKey(0);
}

void
GenericTaxCodeDAO::load()
{
  StartupLoader<QueryGetAllGenericTaxCode, GenericTaxCode, GenericTaxCodeDAO>();
}

bool
GenericTaxCodeDAO::insertDummyObject(std::string& flatKey, ObjectKey& objectKey)
{
  GenericTaxCode* info(new GenericTaxCode);
  GenericTaxCode::dummyData(*info);
  DAOCache::pointer_type ptr = cache().get(IntKey(0));
  bool alreadyExists(false);

  for (std::vector<GenericTaxCode*>::const_iterator bit = ptr->begin(); bit != ptr->end(); ++bit)
  {
    const GenericTaxCode* thisTaxCode((*bit));
    if (thisTaxCode->taxCode() == info->taxCode())
    {
      alreadyExists = true;
      break;
    }
  }

  if (alreadyExists)
  {
    delete info;
  }
  else
  {
    ptr->push_back(info);
    cache().getCacheImpl()->queueDiskPut(IntKey(0), true);
  }

  return true;
}

std::vector<GenericTaxCode*>*
GenericTaxCodeDAO::create(IntKey key)
{
  std::vector<GenericTaxCode*>* ret = new std::vector<GenericTaxCode*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllGenericTaxCode gtc(dbAdapter->getAdapter());
    gtc.findAllGenericTaxCode(*ret);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in GenericTaxCodeDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
GenericTaxCodeDAO::destroy(IntKey key, std::vector<GenericTaxCode*>* recs)
{
  destroyContainer(recs);
}

std::string
GenericTaxCodeDAO::_name("GenericTaxCode");
std::string
GenericTaxCodeDAO::_cacheClass("Taxes");

DAOHelper<GenericTaxCodeDAO>
GenericTaxCodeDAO::_helper(_name);

GenericTaxCodeDAO* GenericTaxCodeDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: GenericTaxCodeHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
GenericTaxCodeHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.GenericTaxCodeHistoricalDAO"));
GenericTaxCodeHistoricalDAO&
GenericTaxCodeHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<GenericTaxCode*>&
GenericTaxCodeHistoricalDAO::get(DeleteList& del,
                                 const TaxCode& key,
                                 const DateTime& date,
                                 const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  IsEffectiveHist<GenericTaxCode> isEffective(date, ticketDate);
  GenericTaxCodeHistoricalKey cacheKey;

  DAOUtils::getDateRange(ticketDate, cacheKey._a, cacheKey._b, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(cacheKey);
  del.copy(ptr);
  std::vector<GenericTaxCode*>* ret = new std::vector<GenericTaxCode*>;
  del.adopt(ret);
  DAOCache::value_type::iterator i = find_if(ptr->begin(), ptr->end(), isEffective);
  while (i != ptr->end())
  {
    if ((*i)->taxCode() == key)
      ret->push_back(*i);
    i = find_if(++i, ptr->end(), isEffective);
  }
  return *ret;
}

std::vector<GenericTaxCode*>*
GenericTaxCodeHistoricalDAO::create(GenericTaxCodeHistoricalKey& key)
{
  std::vector<GenericTaxCode*>* ret = new std::vector<GenericTaxCode*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllGenericTaxCodeHistorical gtc(dbAdapter->getAdapter());
    gtc.findAllGenericTaxCode(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in GenericTaxCodeHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

void
GenericTaxCodeHistoricalDAO::destroy(GenericTaxCodeHistoricalKey key,
                                     std::vector<GenericTaxCode*>* recs)
{
  std::vector<GenericTaxCode*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
GenericTaxCodeHistoricalDAO::_name("GenericTaxCodeHistorical");
std::string
GenericTaxCodeHistoricalDAO::_cacheClass("Taxes");
DAOHelper<GenericTaxCodeHistoricalDAO>
GenericTaxCodeHistoricalDAO::_helper(_name);

GenericTaxCodeHistoricalDAO* GenericTaxCodeHistoricalDAO::_instance = nullptr;

} // namespace tse
