//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/TicketEndorsementsDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetTicketEndorsementsInfo.h"
#include "DBAccess/TicketEndorsementsInfo.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

namespace tse
{
log4cxx::LoggerPtr
TicketEndorsementsDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.TicketEndorsementsDAO"));
TicketEndorsementsDAO&
TicketEndorsementsDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const TicketEndorsementsInfo*
getTicketEndorsementsData(const VendorCode& vendor,
                          int itemNo,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    TicketEndorsementsHistoricalDAO& dao = TicketEndorsementsHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    TicketEndorsementsDAO& dao = TicketEndorsementsDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const TicketEndorsementsInfo*
TicketEndorsementsDAO::get(DeleteList& del,
                           const VendorCode& vendor,
                           int itemNo,
                           const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  TicketEndorsementsKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  if (UNLIKELY(ptr->empty()))
    return nullptr;

  del.copy(ptr);

  IsCurrentG<TicketEndorsementsInfo> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (LIKELY(isCurrent(*iter) && NotInhibit<TicketEndorsementsInfo>(*iter)))
      return *iter;
  }
  return nullptr;
}

std::vector<TicketEndorsementsInfo*>*
TicketEndorsementsDAO::create(TicketEndorsementsKey key)
{
  std::vector<TicketEndorsementsInfo*>* ret = new std::vector<TicketEndorsementsInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTicketEndorsementsInfo tei(dbAdapter->getAdapter());
    tei.findTicketEndorsementsInfo(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TicketEndorsementsDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
TicketEndorsementsDAO::destroy(TicketEndorsementsKey key,
                               std::vector<TicketEndorsementsInfo*>* recs)
{
  std::vector<TicketEndorsementsInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
TicketEndorsementsDAO::compress(const std::vector<TicketEndorsementsInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<TicketEndorsementsInfo*>*
TicketEndorsementsDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<TicketEndorsementsInfo>(compressed);
}

std::string
TicketEndorsementsDAO::_name("TicketEndorsements");
std::string
TicketEndorsementsDAO::_cacheClass("Common");
DAOHelper<TicketEndorsementsDAO>
TicketEndorsementsDAO::_helper(_name);
TicketEndorsementsDAO* TicketEndorsementsDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: TicketEndorsementsHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
TicketEndorsementsHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.TicketEndorsementsHistoricalDAO"));
TicketEndorsementsHistoricalDAO&
TicketEndorsementsHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const TicketEndorsementsInfo*
TicketEndorsementsHistoricalDAO::get(DeleteList& del,
                                     const VendorCode& vendor,
                                     int itemNo,
                                     const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  TicketEndorsementsHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentH<TicketEndorsementsInfo> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter) && NotInhibit<TicketEndorsementsInfo>(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<TicketEndorsementsInfo*>*
TicketEndorsementsHistoricalDAO::create(TicketEndorsementsHistoricalKey key)
{
  std::vector<TicketEndorsementsInfo*>* ret = new std::vector<TicketEndorsementsInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTicketEndorsementsInfoHistorical tei(dbAdapter->getAdapter());
    tei.findTicketEndorsementsInfo(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TicketEndorsementsHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
TicketEndorsementsHistoricalDAO::destroy(TicketEndorsementsHistoricalKey key,
                                         std::vector<TicketEndorsementsInfo*>* recs)
{
  std::vector<TicketEndorsementsInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

TicketEndorsementsHistoricalKey
TicketEndorsementsHistoricalDAO::createKey(const TicketEndorsementsInfo* info,
                                           const DateTime& startDate,
                                           const DateTime& endDate)
{
  return TicketEndorsementsHistoricalKey(info->vendor(), info->itemNo(), startDate, endDate);
}

void
TicketEndorsementsHistoricalDAO::load()
{
  StartupLoaderNoDB<TicketEndorsementsInfo, TicketEndorsementsHistoricalDAO>();
}

sfc::CompressedData*
TicketEndorsementsHistoricalDAO::compress(const std::vector<TicketEndorsementsInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<TicketEndorsementsInfo*>*
TicketEndorsementsHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<TicketEndorsementsInfo>(compressed);
}

std::string
TicketEndorsementsHistoricalDAO::_name("TicketEndorsementsHistorical");
std::string
TicketEndorsementsHistoricalDAO::_cacheClass("Common");
DAOHelper<TicketEndorsementsHistoricalDAO>
TicketEndorsementsHistoricalDAO::_helper(_name);
TicketEndorsementsHistoricalDAO* TicketEndorsementsHistoricalDAO::_instance = nullptr;

} // namespace tse
