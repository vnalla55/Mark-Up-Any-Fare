//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/NegFareSecurityDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/NegFareSecurityInfo.h"
#include "DBAccess/Queries/QueryGetNegFareSecurity.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

namespace tse
{
log4cxx::LoggerPtr
NegFareSecurityDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.NegFareSecurityDAO"));

NegFareSecurityDAO&
NegFareSecurityDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<NegFareSecurityInfo*>&
getNegFareSecurityData(const VendorCode& vendor,
                       int itemNo,
                       DeleteList& deleteList,
                       const DateTime& ticketDate,
                       bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    NegFareSecurityHistoricalDAO& dao = NegFareSecurityHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    NegFareSecurityDAO& dao = NegFareSecurityDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const std::vector<NegFareSecurityInfo*>&
NegFareSecurityDAO::get(DeleteList& del,
                        const VendorCode& vendor,
                        int itemNo,
                        const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  NegFareSecurityKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  /*
  del.copy(ptr);

  std::vector<NegFareSecurityInfo*>* ret = new std::vector<NegFareSecurityInfo*>;
  del.adopt(ret);

  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret),
                 IsNotCurrentG<NegFareSecurityInfo>(ticketDate));

  return *ret;
  */
  return *(applyFilter(del, ptr, IsNotCurrentG<NegFareSecurityInfo>(ticketDate)));
}

std::vector<NegFareSecurityInfo*>*
NegFareSecurityDAO::create(NegFareSecurityKey key)
{
  std::vector<NegFareSecurityInfo*>* ret = new std::vector<NegFareSecurityInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetNegFareSecurity nfs(dbAdapter->getAdapter());
    nfs.findNegFareSecurity(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in NegFareSecurityDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

sfc::CompressedData*
NegFareSecurityDAO::compress(const std::vector<NegFareSecurityInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<NegFareSecurityInfo*>*
NegFareSecurityDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<NegFareSecurityInfo>(compressed);
}

void
NegFareSecurityDAO::destroy(NegFareSecurityKey key, std::vector<NegFareSecurityInfo*>* recs)
{
  std::vector<NegFareSecurityInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

NegFareSecurityKey
NegFareSecurityDAO::createKey(NegFareSecurityInfo* info)
{
  return NegFareSecurityKey(info->vendor(), info->itemNo());
}

void
NegFareSecurityDAO::load()
{
  StartupLoaderNoDB<NegFareSecurityInfo, NegFareSecurityDAO>();
}

std::string
NegFareSecurityDAO::_name("NegFareSecurity");
std::string
NegFareSecurityDAO::_cacheClass("Rules");
DAOHelper<NegFareSecurityDAO>
NegFareSecurityDAO::_helper(_name);
NegFareSecurityDAO* NegFareSecurityDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: NegFareSecurityHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
NegFareSecurityHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.NegFareSecurityHistoricalDAO"));
NegFareSecurityHistoricalDAO&
NegFareSecurityHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<NegFareSecurityInfo*>&
NegFareSecurityHistoricalDAO::get(DeleteList& del,
                                  const VendorCode& vendor,
                                  int itemNo,
                                  const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  NegFareSecurityHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  std::vector<NegFareSecurityInfo*>* ret = new std::vector<NegFareSecurityInfo*>;
  del.adopt(ret);

  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotCurrentH<NegFareSecurityInfo>(ticketDate));

  return *ret;
}

std::vector<NegFareSecurityInfo*>*
NegFareSecurityHistoricalDAO::create(NegFareSecurityHistoricalKey key)
{
  std::vector<NegFareSecurityInfo*>* ret = new std::vector<NegFareSecurityInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get("Historical", true);

  try
  {
    QueryGetNegFareSecurityHistorical nfs(dbAdapter->getAdapter());
    nfs.findNegFareSecurity(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in NegFareSecurityHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

NegFareSecurityHistoricalKey
NegFareSecurityHistoricalDAO::createKey(NegFareSecurityInfo* info,
                                        const DateTime& startDate,
                                        const DateTime& endDate)
{
  return NegFareSecurityHistoricalKey(info->vendor(), info->itemNo(), startDate, endDate);
}

sfc::CompressedData*
NegFareSecurityHistoricalDAO::compress(const std::vector<NegFareSecurityInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<NegFareSecurityInfo*>*
NegFareSecurityHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<NegFareSecurityInfo>(compressed);
}

void
NegFareSecurityHistoricalDAO::load()
{
  StartupLoaderNoDB<NegFareSecurityInfo, NegFareSecurityHistoricalDAO>();
}

void
NegFareSecurityHistoricalDAO::destroy(NegFareSecurityHistoricalKey key,
                                      std::vector<NegFareSecurityInfo*>* recs)
{
  std::vector<NegFareSecurityInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
NegFareSecurityHistoricalDAO::_name("NegFareSecurityHistorical");
std::string
NegFareSecurityHistoricalDAO::_cacheClass("Rules");
DAOHelper<NegFareSecurityHistoricalDAO>
NegFareSecurityHistoricalDAO::_helper(_name);
NegFareSecurityHistoricalDAO* NegFareSecurityHistoricalDAO::_instance = nullptr;

} // namespace tse
