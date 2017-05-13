//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/MiscFareTagDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/MiscFareTag.h"
#include "DBAccess/Queries/QueryGetMiscFareTag.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
MiscFareTagDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.MiscFareTagDAO"));

MiscFareTagDAO&
MiscFareTagDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const MiscFareTag*
getMiscFareTagData(const VendorCode& vendor,
                   int itemNo,
                   DeleteList& deleteList,
                   const DateTime& ticketDate,
                   bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    MiscFareTagHistoricalDAO& dao = MiscFareTagHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    MiscFareTagDAO& dao = MiscFareTagDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const MiscFareTag*
MiscFareTagDAO::get(DeleteList& del,
                    const VendorCode& vendor,
                    int itemNo,
                    const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  MiscFareTagKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentG<MiscFareTag> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (LIKELY(isCurrent(*iter) && NotInhibit<MiscFareTag>(*iter)))
      return *iter;
  }
  return nullptr;
}

std::vector<MiscFareTag*>*
MiscFareTagDAO::create(MiscFareTagKey key)
{
  std::vector<MiscFareTag*>* ret = new std::vector<MiscFareTag*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetMiscFareTag mft(dbAdapter->getAdapter());
    mft.findMiscFareTag(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in MiscFareTagDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
MiscFareTagDAO::destroy(MiscFareTagKey key, std::vector<MiscFareTag*>* recs)
{
  std::vector<MiscFareTag*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

MiscFareTagKey
MiscFareTagDAO::createKey(MiscFareTag* info)
{
  return MiscFareTagKey(info->vendor(), info->itemNo());
}

void
MiscFareTagDAO::load()
{
  StartupLoaderNoDB<MiscFareTag, MiscFareTagDAO>();
}

sfc::CompressedData*
MiscFareTagDAO::compress(const std::vector<MiscFareTag*>* vect) const
{
  return compressVector(vect);
}

std::vector<MiscFareTag*>*
MiscFareTagDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<MiscFareTag>(compressed);
}

std::string
MiscFareTagDAO::_name("MiscFareTag");
std::string
MiscFareTagDAO::_cacheClass("Rules");
DAOHelper<MiscFareTagDAO>
MiscFareTagDAO::_helper(_name);
MiscFareTagDAO* MiscFareTagDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: MiscFareTagHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
MiscFareTagHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.MiscFareTagHistoricalDAO"));
MiscFareTagHistoricalDAO&
MiscFareTagHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const MiscFareTag*
MiscFareTagHistoricalDAO::get(DeleteList& del,
                              const VendorCode& vendor,
                              int itemNo,
                              const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  MiscFareTagHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentH<MiscFareTag> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter) && NotInhibit<MiscFareTag>(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<MiscFareTag*>*
MiscFareTagHistoricalDAO::create(MiscFareTagHistoricalKey key)
{
  std::vector<MiscFareTag*>* ret = new std::vector<MiscFareTag*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetMiscFareTagHistorical mft(dbAdapter->getAdapter());
    mft.findMiscFareTag(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in MiscFareTagHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
MiscFareTagHistoricalDAO::destroy(MiscFareTagHistoricalKey key, std::vector<MiscFareTag*>* recs)
{
  std::vector<MiscFareTag*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
MiscFareTagHistoricalDAO::compress(const std::vector<MiscFareTag*>* vect) const
{
  return compressVector(vect);
}

std::vector<MiscFareTag*>*
MiscFareTagHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<MiscFareTag>(compressed);
}

std::string
MiscFareTagHistoricalDAO::_name("MiscFareTagHistorical");
std::string
MiscFareTagHistoricalDAO::_cacheClass("Rules");
DAOHelper<MiscFareTagHistoricalDAO>
MiscFareTagHistoricalDAO::_helper(_name);
MiscFareTagHistoricalDAO* MiscFareTagHistoricalDAO::_instance = nullptr;

} // namespace tse
