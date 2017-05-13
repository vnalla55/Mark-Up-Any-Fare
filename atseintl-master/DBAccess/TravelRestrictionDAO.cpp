//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/TravelRestrictionDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetTravelRestriction.h"
#include "DBAccess/TravelRestriction.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

namespace tse
{
log4cxx::LoggerPtr
TravelRestrictionDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.TravelRestrictionDAO"));
TravelRestrictionDAO&
TravelRestrictionDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const TravelRestriction*
getTravelRestrictionData(const VendorCode& vendor,
                         int itemNo,
                         DeleteList& deleteList,
                         const DateTime& ticketDate,
                         bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    TravelRestrictionHistoricalDAO& dao = TravelRestrictionHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    TravelRestrictionDAO& dao = TravelRestrictionDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const TravelRestriction*
TravelRestrictionDAO::get(DeleteList& del,
                          const VendorCode& vendor,
                          int itemNo,
                          const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  TravelRestrictionKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  if (UNLIKELY(ptr->empty()))
    return nullptr;

  del.copy(ptr);

  IsCurrentG<TravelRestriction> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (LIKELY(isCurrent(*iter) && NotInhibit<TravelRestriction>(*iter)))
      return *iter;
  }
  return nullptr;
}

std::vector<TravelRestriction*>*
TravelRestrictionDAO::create(TravelRestrictionKey key)
{
  std::vector<TravelRestriction*>* ret = new std::vector<TravelRestriction*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTravelRestriction tr(dbAdapter->getAdapter());
    tr.findTravelRestriction(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TravelRestrictionDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
TravelRestrictionDAO::destroy(TravelRestrictionKey key, std::vector<TravelRestriction*>* recs)
{
  std::vector<TravelRestriction*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

TravelRestrictionKey
TravelRestrictionDAO::createKey(TravelRestriction* info)
{
  return TravelRestrictionKey(info->vendor(), info->itemNo());
}

void
TravelRestrictionDAO::load()
{
  StartupLoaderNoDB<TravelRestriction, TravelRestrictionDAO>();
}

sfc::CompressedData*
TravelRestrictionDAO::compress(const std::vector<TravelRestriction*>* vect) const
{
  return compressVector(vect);
}

std::vector<TravelRestriction*>*
TravelRestrictionDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<TravelRestriction>(compressed);
}

std::string
TravelRestrictionDAO::_name("TravelRestriction");
std::string
TravelRestrictionDAO::_cacheClass("Rules");
DAOHelper<TravelRestrictionDAO>
TravelRestrictionDAO::_helper(_name);
TravelRestrictionDAO* TravelRestrictionDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: TravelRestrictionHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
TravelRestrictionHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.TravelRestrictionHistoricalDAO"));
TravelRestrictionHistoricalDAO&
TravelRestrictionHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const TravelRestriction*
TravelRestrictionHistoricalDAO::get(DeleteList& del,
                                    const VendorCode& vendor,
                                    int itemNo,
                                    const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  TravelRestrictionHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentH<TravelRestriction> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter) && NotInhibit<TravelRestriction>(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<TravelRestriction*>*
TravelRestrictionHistoricalDAO::create(TravelRestrictionHistoricalKey key)
{
  std::vector<TravelRestriction*>* ret = new std::vector<TravelRestriction*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTravelRestrictionHistorical tr(dbAdapter->getAdapter());
    tr.findTravelRestriction(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TravelRestrictionHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
TravelRestrictionHistoricalDAO::destroy(TravelRestrictionHistoricalKey key,
                                        std::vector<TravelRestriction*>* recs)
{
  std::vector<TravelRestriction*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

TravelRestrictionHistoricalKey
TravelRestrictionHistoricalDAO::createKey(const TravelRestriction* info,
                                          const DateTime& startDate,
                                          const DateTime& endDate)
{
  return TravelRestrictionHistoricalKey(info->vendor(), info->itemNo(), startDate, endDate);
}

void
TravelRestrictionHistoricalDAO::load()
{
  StartupLoaderNoDB<TravelRestriction, TravelRestrictionHistoricalDAO>();
}

sfc::CompressedData*
TravelRestrictionHistoricalDAO::compress(const std::vector<TravelRestriction*>* vect) const
{
  return compressVector(vect);
}

std::vector<TravelRestriction*>*
TravelRestrictionHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<TravelRestriction>(compressed);
}

std::string
TravelRestrictionHistoricalDAO::_name("TravelRestrictionHistorical");
std::string
TravelRestrictionHistoricalDAO::_cacheClass("Rules");
DAOHelper<TravelRestrictionHistoricalDAO>
TravelRestrictionHistoricalDAO::_helper(_name);
TravelRestrictionHistoricalDAO* TravelRestrictionHistoricalDAO::_instance = nullptr;

} // namespace tse
