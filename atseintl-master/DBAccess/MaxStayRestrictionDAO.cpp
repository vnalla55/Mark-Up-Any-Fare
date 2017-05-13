//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/MaxStayRestrictionDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/MaxStayRestriction.h"
#include "DBAccess/Queries/QueryGetMaxStayRestriction.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
MaxStayRestrictionDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.MaxStayRestrictionDAO"));

MaxStayRestrictionDAO&
MaxStayRestrictionDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const MaxStayRestriction*
getMaxStayRestrictionData(const VendorCode& vendor,
                          int itemNo,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    MaxStayRestrictionHistoricalDAO& dao = MaxStayRestrictionHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    MaxStayRestrictionDAO& dao = MaxStayRestrictionDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const MaxStayRestriction*
MaxStayRestrictionDAO::get(DeleteList& del,
                           const VendorCode& vendor,
                           int itemNo,
                           const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  MaxStayRestrictionKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  if (UNLIKELY(ptr->empty()))
    return nullptr;

  del.copy(ptr);

  IsCurrentG<MaxStayRestriction> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (LIKELY(isCurrent(*iter) && NotInhibit<MaxStayRestriction>(*iter)))
      return *iter;
  }
  return nullptr;
}

std::vector<MaxStayRestriction*>*
MaxStayRestrictionDAO::create(MaxStayRestrictionKey key)
{
  std::vector<MaxStayRestriction*>* ret = new std::vector<MaxStayRestriction*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetMaxStayRestriction msr(dbAdapter->getAdapter());
    msr.findMaxStayRestriction(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in MaxStayRestrictionDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
MaxStayRestrictionDAO::destroy(MaxStayRestrictionKey key, std::vector<MaxStayRestriction*>* recs)
{
  std::vector<MaxStayRestriction*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

MaxStayRestrictionKey
MaxStayRestrictionDAO::createKey(MaxStayRestriction* info)
{
  return MaxStayRestrictionKey(info->vendor(), info->itemNo());
}

void
MaxStayRestrictionDAO::load()
{
  StartupLoaderNoDB<MaxStayRestriction, MaxStayRestrictionDAO>();
}

sfc::CompressedData*
MaxStayRestrictionDAO::compress(const std::vector<MaxStayRestriction*>* vect) const
{
  return compressVector(vect);
}

std::vector<MaxStayRestriction*>*
MaxStayRestrictionDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<MaxStayRestriction>(compressed);
}

std::string
MaxStayRestrictionDAO::_name("MaxStayRestriction");
std::string
MaxStayRestrictionDAO::_cacheClass("Rules");
DAOHelper<MaxStayRestrictionDAO>
MaxStayRestrictionDAO::_helper(_name);
MaxStayRestrictionDAO* MaxStayRestrictionDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: MaxStayRestrictionHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
MaxStayRestrictionHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.MaxStayRestrictionHistoricalDAO"));
MaxStayRestrictionHistoricalDAO&
MaxStayRestrictionHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const MaxStayRestriction*
MaxStayRestrictionHistoricalDAO::get(DeleteList& del,
                                     const VendorCode& vendor,
                                     int itemNo,
                                     const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  MaxStayRestrictionHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentH<MaxStayRestriction> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter) && NotInhibit<MaxStayRestriction>(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<MaxStayRestriction*>*
MaxStayRestrictionHistoricalDAO::create(MaxStayRestrictionHistoricalKey key)
{
  std::vector<MaxStayRestriction*>* ret = new std::vector<MaxStayRestriction*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetMaxStayRestrictionHistorical msr(dbAdapter->getAdapter());
    msr.findMaxStayRestriction(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in MaxStayRestrictionHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
MaxStayRestrictionHistoricalDAO::destroy(MaxStayRestrictionHistoricalKey key,
                                         std::vector<MaxStayRestriction*>* recs)
{
  std::vector<MaxStayRestriction*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
MaxStayRestrictionHistoricalDAO::compress(const std::vector<MaxStayRestriction*>* vect) const
{
  return compressVector(vect);
}

std::vector<MaxStayRestriction*>*
MaxStayRestrictionHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<MaxStayRestriction>(compressed);
}

std::string
MaxStayRestrictionHistoricalDAO::_name("MaxStayRestrictionHistorical");
std::string
MaxStayRestrictionHistoricalDAO::_cacheClass("Rules");
DAOHelper<MaxStayRestrictionHistoricalDAO>
MaxStayRestrictionHistoricalDAO::_helper(_name);
MaxStayRestrictionHistoricalDAO* MaxStayRestrictionHistoricalDAO::_instance = nullptr;

} // namespace tse
