//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/SvcFeesSecurityDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DAOUtils.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DBHistoryServer.h"
#include "DBAccess/Queries/QueryGetSvcFeesSecurity.h"
#include "DBAccess/SvcFeesSecurityInfo.h"

namespace tse
{
log4cxx::LoggerPtr
SvcFeesSecurityDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SvcFeesSecurityDAO"));

SvcFeesSecurityDAO&
SvcFeesSecurityDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<SvcFeesSecurityInfo*>&
getSvcFeesSecurityData(const VendorCode& vendor,
                       int itemNo,
                       DeleteList& deleteList,
                       const DateTime& ticketDate,
                       bool isHistorical)
{
  if (isHistorical)
  {
    SvcFeesSecurityHistoricalDAO& dao = SvcFeesSecurityHistoricalDAO::instance();

    const std::vector<SvcFeesSecurityInfo*>& ret = dao.get(deleteList, vendor, itemNo, ticketDate);

    return ret;
  }
  else
  {
    SvcFeesSecurityDAO& dao = SvcFeesSecurityDAO::instance();

    const std::vector<SvcFeesSecurityInfo*>& ret = dao.get(deleteList, vendor, itemNo, ticketDate);

    return ret;
  }
}

const std::vector<SvcFeesSecurityInfo*>&
SvcFeesSecurityDAO::get(DeleteList& del,
                        const VendorCode& vendor,
                        int itemNo,
                        const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  SvcFeesSecurityKey key(vendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<SvcFeesSecurityInfo*>* ret = new std::vector<SvcFeesSecurityInfo*>;
  del.adopt(ret);

  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotCurrentG<SvcFeesSecurityInfo>(ticketDate));

  return *ret;
}

std::vector<SvcFeesSecurityInfo*>*
SvcFeesSecurityDAO::create(SvcFeesSecurityKey key)
{
  std::vector<SvcFeesSecurityInfo*>* ret = new std::vector<SvcFeesSecurityInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetSvcFeesSecurity qsfac(dbAdapter->getAdapter());
    qsfac.findSvcFeesSecurityInfo(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in SvcFeesSecurityDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
SvcFeesSecurityDAO::destroy(SvcFeesSecurityKey key, std::vector<SvcFeesSecurityInfo*>* recs)
{
  destroyContainer(recs);
}

SvcFeesSecurityKey
SvcFeesSecurityDAO::createKey(SvcFeesSecurityInfo* info)
{
  return SvcFeesSecurityKey(info->vendor(), info->itemNo());
}

void
SvcFeesSecurityDAO::load()
{
  // Not pre_loading

  StartupLoaderNoDB<SvcFeesSecurityInfo, SvcFeesSecurityDAO>();
}

sfc::CompressedData*
SvcFeesSecurityDAO::compress(const std::vector<SvcFeesSecurityInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<SvcFeesSecurityInfo*>*
SvcFeesSecurityDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<SvcFeesSecurityInfo>(compressed);
}

std::string
SvcFeesSecurityDAO::_name("SvcFeesSecurity");
std::string
SvcFeesSecurityDAO::_cacheClass("Rules");
DAOHelper<SvcFeesSecurityDAO>
SvcFeesSecurityDAO::_helper(_name);
SvcFeesSecurityDAO* SvcFeesSecurityDAO::_instance = nullptr;

///////////////////////////////////////////////////////
// Historical DAO Object
///////////////////////////////////////////////////////
log4cxx::LoggerPtr
SvcFeesSecurityHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SvcFeesSecurityHistDAO"));
SvcFeesSecurityHistoricalDAO&
SvcFeesSecurityHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<SvcFeesSecurityInfo*>&
SvcFeesSecurityHistoricalDAO::get(DeleteList& del,
                                  const VendorCode& vendor,
                                  int itemNo,
                                  const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  SvcFeesSecurityHistoricalKey key(vendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<SvcFeesSecurityInfo*>* ret = new std::vector<SvcFeesSecurityInfo*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotCurrentH<SvcFeesSecurityInfo>(ticketDate));
  return *ret;
}

std::vector<SvcFeesSecurityInfo*>*
SvcFeesSecurityHistoricalDAO::create(SvcFeesSecurityHistoricalKey key)
{
  std::vector<SvcFeesSecurityInfo*>* ret = new std::vector<SvcFeesSecurityInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetSvcFeesSecurityHistorical fnc(dbAdapter->getAdapter());
    fnc.findSvcFeesSecurityInfo(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in SvcFeesSecurityHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
SvcFeesSecurityHistoricalDAO::destroy(SvcFeesSecurityHistoricalKey key,
                                      std::vector<SvcFeesSecurityInfo*>* recs)
{
  std::vector<SvcFeesSecurityInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
SvcFeesSecurityHistoricalDAO::compress(const std::vector<SvcFeesSecurityInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<SvcFeesSecurityInfo*>*
SvcFeesSecurityHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<SvcFeesSecurityInfo>(compressed);
}

std::string
SvcFeesSecurityHistoricalDAO::_name("SvcFeesSecurityHistorical");
std::string
SvcFeesSecurityHistoricalDAO::_cacheClass("Rules");
DAOHelper<SvcFeesSecurityHistoricalDAO>
SvcFeesSecurityHistoricalDAO::_helper(_name);
SvcFeesSecurityHistoricalDAO* SvcFeesSecurityHistoricalDAO::_instance = nullptr;

} // namespace tse
