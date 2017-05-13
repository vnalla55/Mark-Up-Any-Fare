//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/DateOverrideRuleDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DateOverrideRuleItem.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetDateOverrideRule.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
DateOverrideRuleDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.DateOverrideRuleDAO"));

DateOverrideRuleDAO&
DateOverrideRuleDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<DateOverrideRuleItem*>&
getDateOverrideRuleItemData(const VendorCode& vendor,
                            int itemNo,
                            DeleteList& deleteList,
                            const DateTime& ticketDate,
                            bool isHistorical,
                            const DateTime& applDate)
{
  if (UNLIKELY(isHistorical))
  {
    DateOverrideRuleHistoricalDAO& dao = DateOverrideRuleHistoricalDAO::instance();
    if (applDate != DateTime::emptyDate())
    {
      const std::vector<DateOverrideRuleItem*>& ret = dao.get(deleteList, vendor, itemNo, applDate);
      if (ret.size() > 0)
        return ret;
    }
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    DateOverrideRuleDAO& dao = DateOverrideRuleDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const std::vector<DateOverrideRuleItem*>&
DateOverrideRuleDAO::get(DeleteList& del,
                         const VendorCode& vendor,
                         int itemNo,
                         const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  DateOverrideRuleKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  std::vector<DateOverrideRuleItem*>* ret = new std::vector<DateOverrideRuleItem*>;
  del.adopt(ret);

  IsCurrentG<DateOverrideRuleItem> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (LIKELY(isCurrent(*iter) && NotInhibit<DateOverrideRuleItem>(*iter)))
      ret->push_back(*iter);
  }
  return *ret;
}

std::vector<DateOverrideRuleItem*>*
DateOverrideRuleDAO::create(DateOverrideRuleKey key)
{
  std::vector<DateOverrideRuleItem*>* ret = new std::vector<DateOverrideRuleItem*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetDateOverrideRule dor(dbAdapter->getAdapter());
    dor.findDateOverrideRuleItem(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in DateOverrideRuleDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
DateOverrideRuleDAO::destroy(DateOverrideRuleKey key, std::vector<DateOverrideRuleItem*>* recs)
{
  std::vector<DateOverrideRuleItem*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

DateOverrideRuleKey
DateOverrideRuleDAO::createKey(DateOverrideRuleItem* info)
{
  return DateOverrideRuleKey(info->vendor(), info->itemNo());
}

void
DateOverrideRuleDAO::load()
{
  StartupLoaderNoDB<DateOverrideRuleItem, DateOverrideRuleDAO>();
}

sfc::CompressedData*
DateOverrideRuleDAO::compress(const std::vector<DateOverrideRuleItem*>* vect) const
{
  return compressVector(vect);
}

std::vector<DateOverrideRuleItem*>*
DateOverrideRuleDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<DateOverrideRuleItem>(compressed);
}

std::string
DateOverrideRuleDAO::_name("DateOverrideRule");
std::string
DateOverrideRuleDAO::_cacheClass("Rules");
DAOHelper<DateOverrideRuleDAO>
DateOverrideRuleDAO::_helper(_name);
DateOverrideRuleDAO* DateOverrideRuleDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: DateOverrideRuleHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
DateOverrideRuleHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.DateOverrideRuleHistoricalDAO"));
DateOverrideRuleHistoricalDAO&
DateOverrideRuleHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<DateOverrideRuleItem*>&
DateOverrideRuleHistoricalDAO::get(DeleteList& del,
                                   const VendorCode& vendor,
                                   int itemNo,
                                   const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  DateOverrideRuleHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  std::vector<DateOverrideRuleItem*>* ret = new std::vector<DateOverrideRuleItem*>;
  del.adopt(ret);

  IsCurrentH<DateOverrideRuleItem> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter) && NotInhibit<DateOverrideRuleItem>(*iter))
      ret->push_back(*iter);
  }
  return *ret;
}

std::vector<DateOverrideRuleItem*>*
DateOverrideRuleHistoricalDAO::create(DateOverrideRuleHistoricalKey key)
{
  std::vector<DateOverrideRuleItem*>* ret = new std::vector<DateOverrideRuleItem*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetDateOverrideRuleHistorical dor(dbAdapter->getAdapter());
    dor.findDateOverrideRuleItem(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in DateOverrideRuleHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
DateOverrideRuleHistoricalDAO::destroy(DateOverrideRuleHistoricalKey key,
                                       std::vector<DateOverrideRuleItem*>* recs)
{
  std::vector<DateOverrideRuleItem*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
DateOverrideRuleHistoricalDAO::compress(const std::vector<DateOverrideRuleItem*>* vect) const
{
  return compressVector(vect);
}

std::vector<DateOverrideRuleItem*>*
DateOverrideRuleHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<DateOverrideRuleItem>(compressed);
}

std::string
DateOverrideRuleHistoricalDAO::_name("DateOverrideRuleHistorical");
std::string
DateOverrideRuleHistoricalDAO::_cacheClass("Rules");
DAOHelper<DateOverrideRuleHistoricalDAO>
DateOverrideRuleHistoricalDAO::_helper(_name);
DateOverrideRuleHistoricalDAO* DateOverrideRuleHistoricalDAO::_instance = nullptr;

} // namespace tse
