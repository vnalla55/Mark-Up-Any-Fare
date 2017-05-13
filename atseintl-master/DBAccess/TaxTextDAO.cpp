//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
//-------------------------------------------------------------------------------
#include "DBAccess/TaxTextDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetTaxText.h"
#include "DBAccess/TaxCarrierAppl.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
TaxTextDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.TaxTextDAO"));

TaxTextDAO&
TaxTextDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const TaxText*
getTaxTextData(const VendorCode& vendor,
               int itemNo,
               DeleteList& deleteList,
               const DateTime& ticketDate,
               bool isHistorical)
{
  if (isHistorical)
  {
    TaxTextHistoricalDAO& dao = TaxTextHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    TaxTextDAO& dao = TaxTextDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const TaxText*
TaxTextDAO::get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  TaxTextKey key(vendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentG<TaxText> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<TaxText*>*
TaxTextDAO::create(TaxTextKey key)
{
  std::vector<TaxText*>* ret = new std::vector<TaxText*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTaxText cf(dbAdapter->getAdapter());
    cf.findTaxText(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TaxTextDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
TaxTextDAO::destroy(TaxTextKey key, std::vector<TaxText*>* recs)
{
  std::vector<TaxText*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

TaxTextKey
TaxTextDAO::createKey(TaxText* info)
{
  return TaxTextKey(info->vendor(), info->itemNo());
}

void
TaxTextDAO::load()
{
  StartupLoaderNoDB<TaxText, TaxTextDAO>();
}

sfc::CompressedData*
TaxTextDAO::compress(const std::vector<TaxText*>* vect) const
{
  return compressVector(vect);
}

std::vector<TaxText*>*
TaxTextDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<TaxText>(compressed);
}

std::string
TaxTextDAO::_name("TaxText");
std::string
TaxTextDAO::_cacheClass("Rules");
DAOHelper<TaxTextDAO>
TaxTextDAO::_helper(_name);
TaxTextDAO* TaxTextDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: TaxTextHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
TaxTextHistoricalDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.TaxTextHistoricalDAO"));
TaxTextHistoricalDAO&
TaxTextHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const TaxText*
TaxTextHistoricalDAO::get(DeleteList& del,
                          const VendorCode& vendor,
                          int itemNo,
                          const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  TaxTextHistoricalKey key(vendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentH<TaxText> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<TaxText*>*
TaxTextHistoricalDAO::create(TaxTextHistoricalKey key)
{
  std::vector<TaxText*>* ret = new std::vector<TaxText*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTaxTextHistorical cf(dbAdapter->getAdapter());
    cf.findTaxText(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TaxTextHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
TaxTextHistoricalDAO::destroy(TaxTextHistoricalKey key, std::vector<TaxText*>* recs)
{
  std::vector<TaxText*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
TaxTextHistoricalDAO::compress(const std::vector<TaxText*>* vect) const
{
  return compressVector(vect);
}

std::vector<TaxText*>*
TaxTextHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<TaxText>(compressed);
}

std::string
TaxTextHistoricalDAO::_name("TaxTextHistorical");
std::string
TaxTextHistoricalDAO::_cacheClass("Rules");
DAOHelper<TaxTextHistoricalDAO>
TaxTextHistoricalDAO::_helper(_name);
TaxTextHistoricalDAO* TaxTextHistoricalDAO::_instance = nullptr;

} // namespace tse
