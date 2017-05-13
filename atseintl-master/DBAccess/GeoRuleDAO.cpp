//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/GeoRuleDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/GeoRuleItem.h"
#include "DBAccess/Queries/QueryGetGeoRule.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
GeoRuleDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.GeoRuleDAO"));

GeoRuleDAO&
GeoRuleDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<tse::GeoRuleItem*>&
getGeoRuleItemData(const VendorCode& vendor,
                   int itemNo,
                   DeleteList& deleteList,
                   const DateTime& ticketDate,
                   bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    GeoRuleHistoricalDAO& dao = GeoRuleHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    GeoRuleDAO& dao = GeoRuleDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const std::vector<GeoRuleItem*>&
GeoRuleDAO::get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  GeoRuleKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  /*
  del.copy(ptr);

  std::vector<GeoRuleItem*>* ret = new std::vector<GeoRuleItem*>;
  del.adopt(ret);

  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret),
                 IsNotCurrentG<GeoRuleItem>(ticketDate));

  return *ret;
  */
  return *(applyFilter(del, ptr, IsNotCurrentG<GeoRuleItem>(ticketDate)));
}

std::vector<GeoRuleItem*>*
GeoRuleDAO::create(GeoRuleKey key)
{
  std::vector<GeoRuleItem*>* ret = new std::vector<GeoRuleItem*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetGeoRule gr(dbAdapter->getAdapter());
    gr.findGeoRuleItem(*ret, key._a, key._b);
    ret->erase(std::remove_if(ret->begin(), ret->end(), Inhibit<GeoRuleItem>), ret->end());
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in GeoRuleDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
GeoRuleDAO::destroy(GeoRuleKey key, std::vector<GeoRuleItem*>* recs)
{
  std::vector<GeoRuleItem*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
GeoRuleDAO::_name("GeoRule");
std::string
GeoRuleDAO::_cacheClass("Rules");
DAOHelper<GeoRuleDAO>
GeoRuleDAO::_helper(_name);
GeoRuleDAO* GeoRuleDAO::_instance = nullptr;

GeoRuleKey
GeoRuleDAO::createKey(GeoRuleItem* info)
{
  return GeoRuleKey(info->vendor(), info->itemNo());
}

void
GeoRuleDAO::load()
{
  StartupLoaderNoDB<GeoRuleItem, GeoRuleDAO>();
}

sfc::CompressedData*
GeoRuleDAO::compress(const std::vector<GeoRuleItem*>* vect) const
{
  return compressVector(vect);
}

std::vector<GeoRuleItem*>*
GeoRuleDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<GeoRuleItem>(compressed);
}

// --------------------------------------------------
// Historical DAO: GeoRuleHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
GeoRuleHistoricalDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.GeoRuleHistoricalDAO"));
GeoRuleHistoricalDAO&
GeoRuleHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<GeoRuleItem*>&
GeoRuleHistoricalDAO::get(DeleteList& del,
                          const VendorCode& vendor,
                          int itemNo,
                          const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  GeoRuleHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  std::vector<GeoRuleItem*>* ret = new std::vector<GeoRuleItem*>;
  del.adopt(ret);

  remove_copy_if(
      ptr->begin(), ptr->end(), back_inserter(*ret), IsNotCurrentH<GeoRuleItem>(ticketDate));

  return *ret;
}

std::vector<GeoRuleItem*>*
GeoRuleHistoricalDAO::create(GeoRuleHistoricalKey key)
{
  std::vector<GeoRuleItem*>* ret = new std::vector<GeoRuleItem*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetGeoRuleHistorical gr(dbAdapter->getAdapter());
    gr.findGeoRuleItem(*ret, key._a, key._b, key._c, key._d);
    ret->erase(std::remove_if(ret->begin(), ret->end(), Inhibit<GeoRuleItem>), ret->end());
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in GeoRuleHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
GeoRuleHistoricalDAO::destroy(GeoRuleHistoricalKey key, std::vector<GeoRuleItem*>* recs)
{
  std::vector<GeoRuleItem*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
GeoRuleHistoricalDAO::compress(const std::vector<GeoRuleItem*>* vect) const
{
  return compressVector(vect);
}

std::vector<GeoRuleItem*>*
GeoRuleHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<GeoRuleItem>(compressed);
}

std::string
GeoRuleHistoricalDAO::_name("GeoRuleHistorical");
std::string
GeoRuleHistoricalDAO::_cacheClass("Rules");
DAOHelper<GeoRuleHistoricalDAO>
GeoRuleHistoricalDAO::_helper(_name);
GeoRuleHistoricalDAO* GeoRuleHistoricalDAO::_instance = nullptr;

} // namespace tse
