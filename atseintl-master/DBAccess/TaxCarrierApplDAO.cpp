//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
//-------------------------------------------------------------------------------
#include "DBAccess/TaxCarrierApplDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetTaxCarrierAppl.h"
#include "DBAccess/TaxCarrierAppl.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
TaxCarrierApplDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.TaxCarrierApplDAO"));

TaxCarrierApplDAO&
TaxCarrierApplDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const TaxCarrierAppl*
getTaxCarrierApplData(const VendorCode& vendor,
                      int itemNo,
                      DeleteList& deleteList,
                      const DateTime& ticketDate,
                      bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    TaxCarrierApplHistoricalDAO& dao = TaxCarrierApplHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    TaxCarrierApplDAO& dao = TaxCarrierApplDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const TaxCarrierAppl*
TaxCarrierApplDAO::get(DeleteList& del,
                       const VendorCode& vendor,
                       int itemNo,
                       const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  TaxCarrierApplKey key(vendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  if (UNLIKELY(ptr->empty()))
    return nullptr;

  del.copy(ptr);

  IsCurrentG<TaxCarrierAppl> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (LIKELY(isCurrent(*iter)))
      return *iter;
  }
  return nullptr;
}

std::vector<TaxCarrierAppl*>*
TaxCarrierApplDAO::create(TaxCarrierApplKey key)
{
  std::vector<TaxCarrierAppl*>* ret = new std::vector<TaxCarrierAppl*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTaxCarrierAppl cf(dbAdapter->getAdapter());
    cf.findTaxCarrierAppl(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TaxCarrierApplDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
TaxCarrierApplDAO::destroy(TaxCarrierApplKey key, std::vector<TaxCarrierAppl*>* recs)
{
  std::vector<TaxCarrierAppl*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

TaxCarrierApplKey
TaxCarrierApplDAO::createKey(TaxCarrierAppl* info)
{
  return TaxCarrierApplKey(info->vendor(), info->itemNo());
}

void
TaxCarrierApplDAO::load()
{
  StartupLoaderNoDB<TaxCarrierAppl, TaxCarrierApplDAO>();
}

sfc::CompressedData*
TaxCarrierApplDAO::compress(const std::vector<TaxCarrierAppl*>* vect) const
{
  return compressVector(vect);
}

std::vector<TaxCarrierAppl*>*
TaxCarrierApplDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<TaxCarrierAppl>(compressed);
}

std::string
TaxCarrierApplDAO::_name("TaxCarrierAppl");
std::string
TaxCarrierApplDAO::_cacheClass("Rules");
DAOHelper<TaxCarrierApplDAO>
TaxCarrierApplDAO::_helper(_name);
TaxCarrierApplDAO* TaxCarrierApplDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: TaxCarrierApplHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
TaxCarrierApplHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.TaxCarrierApplHistoricalDAO"));
TaxCarrierApplHistoricalDAO&
TaxCarrierApplHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const TaxCarrierAppl*
TaxCarrierApplHistoricalDAO::get(DeleteList& del,
                                 const VendorCode& vendor,
                                 int itemNo,
                                 const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  TaxCarrierApplHistoricalKey key(vendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentH<TaxCarrierAppl> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<TaxCarrierAppl*>*
TaxCarrierApplHistoricalDAO::create(TaxCarrierApplHistoricalKey key)
{
  std::vector<TaxCarrierAppl*>* ret = new std::vector<TaxCarrierAppl*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTaxCarrierApplHistorical cf(dbAdapter->getAdapter());
    cf.findTaxCarrierAppl(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TaxCarrierApplHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
TaxCarrierApplHistoricalDAO::destroy(TaxCarrierApplHistoricalKey key,
                                     std::vector<TaxCarrierAppl*>* recs)
{
  std::vector<TaxCarrierAppl*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
TaxCarrierApplHistoricalDAO::compress(const std::vector<TaxCarrierAppl*>* vect) const
{
  return compressVector(vect);
}

std::vector<TaxCarrierAppl*>*
TaxCarrierApplHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<TaxCarrierAppl>(compressed);
}

std::string
TaxCarrierApplHistoricalDAO::_name("TaxCarrierApplHistorical");
std::string
TaxCarrierApplHistoricalDAO::_cacheClass("Rules");
DAOHelper<TaxCarrierApplHistoricalDAO>
TaxCarrierApplHistoricalDAO::_helper(_name);
TaxCarrierApplHistoricalDAO* TaxCarrierApplHistoricalDAO::_instance = nullptr;

} // namespace tse
