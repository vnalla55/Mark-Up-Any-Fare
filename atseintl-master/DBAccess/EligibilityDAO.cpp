//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/EligibilityDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/EligibilityInfo.h"
#include "DBAccess/Queries/QueryGetEligibility.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
EligibilityDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.EligibilityDAO"));

EligibilityDAO&
EligibilityDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const EligibilityInfo*
getEligibilityData(const VendorCode& vendor,
                   int itemNo,
                   DeleteList& deleteList,
                   const DateTime& ticketDate,
                   bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    EligibilityHistoricalDAO& dao = EligibilityHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    EligibilityDAO& dao = EligibilityDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const EligibilityInfo*
EligibilityDAO::get(DeleteList& del,
                    const VendorCode& vendor,
                    int itemNo,
                    const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  EligibilityKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentG<EligibilityInfo> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (LIKELY(isCurrent(*iter) && NotInhibit<EligibilityInfo>(*iter)))
      return *iter;
  }
  return nullptr;
}

std::vector<const EligibilityInfo*>*
EligibilityDAO::create(EligibilityKey key)
{
  std::vector<const EligibilityInfo*>* ret = new std::vector<const EligibilityInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetEligibility elg(dbAdapter->getAdapter());
    elg.findEligibility(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in EligibilityDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
EligibilityDAO::destroy(EligibilityKey key, std::vector<const EligibilityInfo*>* recs)
{
  destroyContainer(recs);
}

EligibilityKey
EligibilityDAO::createKey(const EligibilityInfo* info)
{
  return EligibilityKey(info->vendor(), info->itemNo());
}

void
EligibilityDAO::load()
{
  StartupLoaderNoDB<EligibilityInfo, EligibilityDAO>();
}

sfc::CompressedData*
EligibilityDAO::compress(const std::vector<const EligibilityInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<const EligibilityInfo*>*
EligibilityDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<const EligibilityInfo>(compressed);
}

std::string
EligibilityDAO::_name("Eligibility");
std::string
EligibilityDAO::_cacheClass("Rules");
DAOHelper<EligibilityDAO>
EligibilityDAO::_helper(_name);
EligibilityDAO* EligibilityDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: EligibilityHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
EligibilityHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.EligibilityHistoricalDAO"));
EligibilityHistoricalDAO&
EligibilityHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const EligibilityInfo*
EligibilityHistoricalDAO::get(DeleteList& del,
                              const VendorCode& vendor,
                              int itemNo,
                              const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  EligibilityHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentH<EligibilityInfo> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter) && NotInhibit<EligibilityInfo>(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<const EligibilityInfo*>*
EligibilityHistoricalDAO::create(EligibilityHistoricalKey key)
{
  std::vector<const EligibilityInfo*>* ret = new std::vector<const EligibilityInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetEligibilityHistorical elg(dbAdapter->getAdapter());
    elg.findEligibility(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in EligibilityHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
EligibilityHistoricalDAO::destroy(EligibilityHistoricalKey key,
                                  std::vector<const EligibilityInfo*>* recs)
{
  std::vector<const EligibilityInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i; // lint !e605
  delete recs;
}

sfc::CompressedData*
EligibilityHistoricalDAO::compress(const std::vector<const EligibilityInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<const EligibilityInfo*>*
EligibilityHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<const EligibilityInfo>(compressed);
}

std::string
EligibilityHistoricalDAO::_name("EligibilityHistorical");
std::string
EligibilityHistoricalDAO::_cacheClass("Rules");
DAOHelper<EligibilityHistoricalDAO>
EligibilityHistoricalDAO::_helper(_name);
EligibilityHistoricalDAO* EligibilityHistoricalDAO::_instance = nullptr;

} // namespace tse
