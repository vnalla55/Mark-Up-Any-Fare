//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/NegFareRestDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/NegFareRest.h"
#include "DBAccess/Queries/QueryGetNegFareRest.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
NegFareRestDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.NegFareRestDAO"));

NegFareRestDAO&
NegFareRestDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const NegFareRest*
getNegFareRestData(const VendorCode& vendor,
                   int itemNo,
                   DeleteList& deleteList,
                   const DateTime& ticketDate,
                   bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    NegFareRestHistoricalDAO& dao = NegFareRestHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    NegFareRestDAO& dao = NegFareRestDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const NegFareRest*
NegFareRestDAO::get(DeleteList& del,
                    const VendorCode& vendor,
                    int itemNo,
                    const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  NegFareRestKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  if (UNLIKELY(ptr->empty()))
    return 0;

  del.copy(ptr);

  IsCurrentG<NegFareRest> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (LIKELY(isCurrent(*iter) && NotInhibit<NegFareRest>(*iter)))
      return *iter;
  }
  return nullptr;
}

std::vector<NegFareRest*>*
NegFareRestDAO::create(NegFareRestKey key)
{
  std::vector<NegFareRest*>* ret = new std::vector<NegFareRest*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetNegFareRest nfr(dbAdapter->getAdapter());
    nfr.findNegFareRest(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in NegFareRestDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
NegFareRestDAO::destroy(NegFareRestKey key, std::vector<NegFareRest*>* recs)
{
  std::vector<NegFareRest*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

NegFareRestKey
NegFareRestDAO::createKey(NegFareRest* info)
{
  return NegFareRestKey(info->vendor(), info->itemNo());
}

void
NegFareRestDAO::load()
{
  StartupLoaderNoDB<NegFareRest, NegFareRestDAO>();
}

sfc::CompressedData*
NegFareRestDAO::compress(const std::vector<NegFareRest*>* vect) const
{
  return compressVector(vect);
}

std::vector<NegFareRest*>*
NegFareRestDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<NegFareRest>(compressed);
}

std::string
NegFareRestDAO::_name("NegFareRest");
std::string
NegFareRestDAO::_cacheClass("Rules");
DAOHelper<NegFareRestDAO>
NegFareRestDAO::_helper(_name);
NegFareRestDAO* NegFareRestDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: NegFareRestHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
NegFareRestHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.NegFareRestHistoricalDAO"));
NegFareRestHistoricalDAO&
NegFareRestHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const NegFareRest*
NegFareRestHistoricalDAO::get(DeleteList& del,
                              const VendorCode& vendor,
                              int itemNo,
                              const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  NegFareRestHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentH<NegFareRest> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter) && NotInhibit<NegFareRest>(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<NegFareRest*>*
NegFareRestHistoricalDAO::create(NegFareRestHistoricalKey key)
{
  std::vector<NegFareRest*>* ret = new std::vector<NegFareRest*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get("Historical", true);

  try
  {
    QueryGetNegFareRestHistorical nfr(dbAdapter->getAdapter());
    nfr.findNegFareRest(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in NegFareRestHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

NegFareRestHistoricalKey
NegFareRestHistoricalDAO::createKey(NegFareRest* info,
                                    const DateTime& startDate,
                                    const DateTime& endDate)
{
  return NegFareRestHistoricalKey(info->vendor(), info->itemNo(), startDate, endDate);
}

void
NegFareRestHistoricalDAO::load()
{
  StartupLoaderNoDB<NegFareRest, NegFareRestHistoricalDAO>();
}

void
NegFareRestHistoricalDAO::destroy(NegFareRestHistoricalKey key, std::vector<NegFareRest*>* recs)
{
  std::vector<NegFareRest*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
NegFareRestHistoricalDAO::compress(const std::vector<NegFareRest*>* vect) const
{
  return compressVector(vect);
}

std::vector<NegFareRest*>*
NegFareRestHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<NegFareRest>(compressed);
}

std::string
NegFareRestHistoricalDAO::_name("NegFareRestHistorical");
std::string
NegFareRestHistoricalDAO::_cacheClass("Rules");
DAOHelper<NegFareRestHistoricalDAO>
NegFareRestHistoricalDAO::_helper(_name);
NegFareRestHistoricalDAO* NegFareRestHistoricalDAO::_instance = nullptr;

} // namespace tse
