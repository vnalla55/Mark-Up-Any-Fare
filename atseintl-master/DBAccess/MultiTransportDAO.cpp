//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#include "DBAccess/MultiTransportDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/MultiTransport.h"
#include "DBAccess/Queries/QueryGetMultiTransport.h"

namespace tse
{
log4cxx::LoggerPtr
MultiTransportDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.MultiTransportDAO"));

MultiTransportDAO&
MultiTransportDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<MultiTransport*>&
MultiTransportDAO::get(DeleteList& del, const LocCode& key, const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(LocCodeKey(key));
  /*
  del.copy(ptr);
  std::vector<MultiTransport*>* ret = new std::vector<MultiTransport*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret),
  IsNotCurrentG<MultiTransport>(ticketDate));
  return *ret;
  */
  return *(applyFilter(del, ptr, IsNotCurrentG<MultiTransport>(ticketDate)));
}

LocCodeKey
MultiTransportDAO::createKey(MultiTransport* info)
{
  return LocCodeKey(info->multitranscity());
}

std::vector<MultiTransport*>*
MultiTransportDAO::create(LocCodeKey key)
{
  std::vector<MultiTransport*>* ret = new std::vector<MultiTransport*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetMultiTransport mt(dbAdapter->getAdapter());
    mt.findMultiTransport(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in MultiTransportDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
MultiTransportDAO::destroy(LocCodeKey key, std::vector<MultiTransport*>* recs)
{
  destroyContainer(recs);
}

void
MultiTransportDAO::load()
{
  StartupLoader<QueryGetAllMultiTransport, MultiTransport, MultiTransportDAO>();
}

sfc::CompressedData*
MultiTransportDAO::compress(const std::vector<MultiTransport*>* vect) const
{
  return compressVector(vect);
}

std::vector<MultiTransport*>*
MultiTransportDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<MultiTransport>(compressed);
}

std::string
MultiTransportDAO::_name("MultiTransport");
std::string
MultiTransportDAO::_cacheClass("Common");

DAOHelper<MultiTransportDAO>
MultiTransportDAO::_helper(_name);

MultiTransportDAO* MultiTransportDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: MultiTransportHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
MultiTransportHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.MultiTransportHistoricalDAO"));
MultiTransportHistoricalDAO&
MultiTransportHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

std::vector<MultiTransport*>&
MultiTransportHistoricalDAO::get(DeleteList& del, const LocCode& key, const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  std::vector<MultiTransport*>* ret = new std::vector<MultiTransport*>;
  del.adopt(ret);
  MultiTransportHistoricalKey cacheKey(key);
  DAOUtils::getDateRange(ticketDate, cacheKey._b, cacheKey._c, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(cacheKey);
  del.copy(ptr);
  remove_copy_if(
      ptr->begin(), ptr->end(), back_inserter(*ret), IsNotCurrentH<MultiTransport>(ticketDate));
  return *ret;
}

std::vector<MultiTransport*>*
MultiTransportHistoricalDAO::create(MultiTransportHistoricalKey key)
{
  std::vector<MultiTransport*>* ret = new std::vector<MultiTransport*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetMultiTransportHistorical mt(dbAdapter->getAdapter());
    mt.findMultiTransport(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in MultiTransportHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
MultiTransportHistoricalDAO::destroy(MultiTransportHistoricalKey key,
                                     std::vector<MultiTransport*>* recs)
{
  destroyContainer(recs);
}

sfc::CompressedData*
MultiTransportHistoricalDAO::compress(const std::vector<MultiTransport*>* vect) const
{
  return compressVector(vect);
}

std::vector<MultiTransport*>*
MultiTransportHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<MultiTransport>(compressed);
}

std::string
MultiTransportHistoricalDAO::_name("MultiTransportHistorical");
std::string
MultiTransportHistoricalDAO::_cacheClass("Common");
DAOHelper<MultiTransportHistoricalDAO>
MultiTransportHistoricalDAO::_helper(_name);

MultiTransportHistoricalDAO* MultiTransportHistoricalDAO::_instance = nullptr;

} // namespace tse
