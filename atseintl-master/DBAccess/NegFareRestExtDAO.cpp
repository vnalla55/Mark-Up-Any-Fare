//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/NegFareRestExtDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/NegFareRestExt.h"
#include "DBAccess/Queries/QueryGetNegFareRestExt.h"
#include "DBAccess/VendorCrossRefDAO.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
NegFareRestExtDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.NegFareRestExtDAO"));

NegFareRestExtDAO&
NegFareRestExtDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const NegFareRestExt*
getNegFareRestExtData(const VendorCode& vendor,
                      int itemNo,
                      DeleteList& deleteList,
                      const DateTime& ticketDate,
                      bool isHistorical)
{
  if (isHistorical)
  {
    NegFareRestExtHistoricalDAO& dao = NegFareRestExtHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    NegFareRestExtDAO& dao = NegFareRestExtDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const NegFareRestExt*
NegFareRestExtDAO::get(DeleteList& del,
                       const VendorCode& vendor,
                       int itemNo,
                       const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  NegFareRestExtKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentG<NegFareRestExt> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<NegFareRestExt*>*
NegFareRestExtDAO::create(NegFareRestExtKey key)
{
  std::vector<NegFareRestExt*>* ret = new std::vector<NegFareRestExt*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetNegFareRestExt nfrt(dbAdapter->getAdapter());
    nfrt.findNegFareRestExt(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in NegFareRestExtDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
NegFareRestExtDAO::destroy(NegFareRestExtKey key, std::vector<NegFareRestExt*>* recs)
{
  std::vector<NegFareRestExt*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

NegFareRestExtKey
NegFareRestExtDAO::createKey(NegFareRestExt* info)
{
  return NegFareRestExtKey(info->vendor(), info->itemNo());
}

void
NegFareRestExtDAO::load()
{
  StartupLoaderNoDB<NegFareRestExt, NegFareRestExtDAO>();
}

sfc::CompressedData*
NegFareRestExtDAO::compress(const std::vector<NegFareRestExt*>* vect) const
{
  return compressVector(vect);
}

std::vector<NegFareRestExt*>*
NegFareRestExtDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<NegFareRestExt>(compressed);
}

std::string
NegFareRestExtDAO::_name("NegFareRestExt");
std::string
NegFareRestExtDAO::_cacheClass("Rules");
DAOHelper<NegFareRestExtDAO>
NegFareRestExtDAO::_helper(_name);
NegFareRestExtDAO* NegFareRestExtDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: NegFareRestExtHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
NegFareRestExtHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.NegFareRestExtHistoricalDAO"));
NegFareRestExtHistoricalDAO&
NegFareRestExtHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const NegFareRestExt*
NegFareRestExtHistoricalDAO::get(DeleteList& del,
                                 const VendorCode& vendor,
                                 int itemNo,
                                 const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  NegFareRestExtHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentH<NegFareRestExt> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<NegFareRestExt*>*
NegFareRestExtHistoricalDAO::create(NegFareRestExtHistoricalKey key)
{
  std::vector<NegFareRestExt*>* ret = new std::vector<NegFareRestExt*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get("Historical", true);

  try
  {
    QueryGetNegFareRestExtHistorical nfrt(dbAdapter->getAdapter());
    nfrt.findNegFareRestExt(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in NegFareRestExtHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
NegFareRestExtHistoricalDAO::destroy(NegFareRestExtHistoricalKey key,
                                     std::vector<NegFareRestExt*>* recs)
{
  std::vector<NegFareRestExt*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
NegFareRestExtHistoricalDAO::compress(const std::vector<NegFareRestExt*>* vect) const
{
  return compressVector(vect);
}

std::vector<NegFareRestExt*>*
NegFareRestExtHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<NegFareRestExt>(compressed);
}

std::string
NegFareRestExtHistoricalDAO::_name("NegFareRestExtHistorical");
std::string
NegFareRestExtHistoricalDAO::_cacheClass("Rules");
DAOHelper<NegFareRestExtHistoricalDAO>
NegFareRestExtHistoricalDAO::_helper(_name);
NegFareRestExtHistoricalDAO* NegFareRestExtHistoricalDAO::_instance = nullptr;

} // namespace tse
