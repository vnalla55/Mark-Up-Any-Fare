//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DBAccess/BaseFareRuleDAO.h"

#include "Common/Logger.h"
#include "DBAccess/BaseFareRule.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetTable989.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

#include <algorithm>
#include <functional>

namespace tse
{

log4cxx::LoggerPtr
BaseFareRuleDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.BaseFareRuleDAO"));

BaseFareRuleDAO&
BaseFareRuleDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<const BaseFareRule*>&
getBaseFareRuleData(const VendorCode& vendor,
                    int itemNo,
                    const DateTime& date,
                    DeleteList& deleteList,
                    const DateTime& ticketDate,
                    bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    BaseFareRuleHistoricalDAO& dao = BaseFareRuleHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, date, ticketDate);
  }
  else
  {
    BaseFareRuleDAO& dao = BaseFareRuleDAO::instance();
    return dao.get(deleteList, vendor, itemNo, date, ticketDate);
  }
}

const std::vector<const BaseFareRule*>&
BaseFareRuleDAO::get(DeleteList& del,
                     const VendorCode& vendor,
                     int itemNo,
                     const DateTime& date,
                     const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);

  BaseFareRuleKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  return *(applyFilter(del, ptr, IsNotCurrentG<BaseFareRule>(ticketDate)));
}

std::vector<const BaseFareRule*>*
BaseFareRuleDAO::create(BaseFareRuleKey key)
{
  std::vector<const BaseFareRule*>* ret = new std::vector<const BaseFareRule*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTable989 t989(dbAdapter->getAdapter());
    t989.findBaseFareRule(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in BaseFareRuleDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
BaseFareRuleDAO::destroy(BaseFareRuleKey key, std::vector<const BaseFareRule*>* recs)
{
  std::vector<const BaseFareRule*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

BaseFareRuleKey
BaseFareRuleDAO::createKey(const BaseFareRule* info)
{
  return BaseFareRuleKey(info->vendor(), info->itemNo());
}

void
BaseFareRuleDAO::load()
{
  StartupLoaderNoDB<const BaseFareRule, BaseFareRuleDAO>();
}

sfc::CompressedData*
BaseFareRuleDAO::compress(const std::vector<const BaseFareRule*>* vect) const
{
  return compressVector(vect);
}

std::vector<const BaseFareRule*>*
BaseFareRuleDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<const BaseFareRule>(compressed);
}

std::string
BaseFareRuleDAO::_name("BaseFareRule");
std::string
BaseFareRuleDAO::_cacheClass("Rules");
DAOHelper<BaseFareRuleDAO>
BaseFareRuleDAO::_helper(_name);
BaseFareRuleDAO* BaseFareRuleDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: BaseFareRuleHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
BaseFareRuleHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.BaseFareRuleHistoricalDAO"));
BaseFareRuleHistoricalDAO&
BaseFareRuleHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<const BaseFareRule*>&
BaseFareRuleHistoricalDAO::get(DeleteList& del,
                               const VendorCode& vendor,
                               int itemNo,
                               const DateTime& date,
                               const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);

  BaseFareRuleHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<const BaseFareRule*>* ret = new std::vector<const BaseFareRule*>;
  del.adopt(ret);
  remove_copy_if(
      ptr->begin(), ptr->end(), back_inserter(*ret), IsNotCurrentH<BaseFareRule>(ticketDate));
  ret->erase(std::remove_if(ret->begin(), ret->end(), Inhibit<BaseFareRule>), ret->end());
  return *ret;
}

std::vector<const BaseFareRule*>*
BaseFareRuleHistoricalDAO::create(BaseFareRuleHistoricalKey key)
{
  std::vector<const BaseFareRule*>* ret = new std::vector<const BaseFareRule*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get("Historical", true);
  try
  {
    QueryGetTable989Historical t989(dbAdapter->getAdapter());
    t989.findBaseFareRule(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in BaseFareRuleHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
BaseFareRuleHistoricalDAO::destroy(BaseFareRuleHistoricalKey key,
                                   std::vector<const BaseFareRule*>* recs)
{
  std::vector<const BaseFareRule*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
BaseFareRuleHistoricalDAO::compress(const std::vector<const BaseFareRule*>* vect) const
{
  return compressVector(vect);
}

std::vector<const BaseFareRule*>*
BaseFareRuleHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<const BaseFareRule>(compressed);
}

std::string
BaseFareRuleHistoricalDAO::_name("BaseFareRuleHistorical");
std::string
BaseFareRuleHistoricalDAO::_cacheClass("Rules");
DAOHelper<BaseFareRuleHistoricalDAO>
BaseFareRuleHistoricalDAO::_helper(_name);

BaseFareRuleHistoricalDAO* BaseFareRuleHistoricalDAO::_instance = nullptr;

} // namespace tse
