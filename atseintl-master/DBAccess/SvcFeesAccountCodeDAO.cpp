//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/SvcFeesAccountCodeDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DAOUtils.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DBHistoryServer.h"
#include "DBAccess/Queries/QueryGetSvcFeesAccountCode.h"
#include "DBAccess/SvcFeesAccCodeInfo.h"

namespace tse
{
log4cxx::LoggerPtr
SvcFeesAccountCodeDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SvcFeesAccountCodeDAO"));

SvcFeesAccountCodeDAO&
SvcFeesAccountCodeDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<SvcFeesAccCodeInfo*>&
getSvcFeesAccountCodeData(const VendorCode& vendor,
                          int itemNo,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    SvcFeesAccountCodeHistoricalDAO& dao = SvcFeesAccountCodeHistoricalDAO::instance();

    const std::vector<SvcFeesAccCodeInfo*>& ret = dao.get(deleteList, vendor, itemNo, ticketDate);

    return ret;
  }
  else
  {
    SvcFeesAccountCodeDAO& dao = SvcFeesAccountCodeDAO::instance();

    const std::vector<SvcFeesAccCodeInfo*>& ret = dao.get(deleteList, vendor, itemNo, ticketDate);

    return ret;
  }
}

const std::vector<SvcFeesAccCodeInfo*>&
SvcFeesAccountCodeDAO::get(DeleteList& del,
                           const VendorCode& vendor,
                           int itemNo,
                           const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  SvcFeesAccountCodeKey key(vendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<SvcFeesAccCodeInfo*>* ret = new std::vector<SvcFeesAccCodeInfo*>;
  del.adopt(ret);
  remove_copy_if(
      ptr->begin(), ptr->end(), back_inserter(*ret), IsNotCurrentG<SvcFeesAccCodeInfo>(ticketDate));
  return *ret;
}

std::vector<SvcFeesAccCodeInfo*>*
SvcFeesAccountCodeDAO::create(SvcFeesAccountCodeKey key)
{
  std::vector<SvcFeesAccCodeInfo*>* ret = new std::vector<SvcFeesAccCodeInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetSvcFeesAccountCode qsfac(dbAdapter->getAdapter());
    qsfac.findSvcFeesAccCodeInfo(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in SvcFeesAccountCodeDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
SvcFeesAccountCodeDAO::destroy(SvcFeesAccountCodeKey key, std::vector<SvcFeesAccCodeInfo*>* recs)
{
  destroyContainer(recs);
}

SvcFeesAccountCodeKey
SvcFeesAccountCodeDAO::createKey(SvcFeesAccCodeInfo* info)
{
  return SvcFeesAccountCodeKey(info->vendor(), info->itemNo());
}

void
SvcFeesAccountCodeDAO::load()
{
  // StartupLoader<QueryGetSvcFeesAccountCode,SvcFeesAccCodeInfo,SvcFeesAccountCodeDAO>() ;
  // Not pre_loading
  StartupLoaderNoDB<SvcFeesAccCodeInfo, SvcFeesAccountCodeDAO>();
}

std::string
SvcFeesAccountCodeDAO::_name("SvcFeesAccountCode");
std::string
SvcFeesAccountCodeDAO::_cacheClass("Rules");
DAOHelper<SvcFeesAccountCodeDAO>
SvcFeesAccountCodeDAO::_helper(_name);
SvcFeesAccountCodeDAO* SvcFeesAccountCodeDAO::_instance = nullptr;

///////////////////////////////////////////////////////
// Historical DAO Object
///////////////////////////////////////////////////////
log4cxx::LoggerPtr
SvcFeesAccountCodeHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SvcFeesAccountCodeHistDAO"));
SvcFeesAccountCodeHistoricalDAO&
SvcFeesAccountCodeHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<SvcFeesAccCodeInfo*>&
SvcFeesAccountCodeHistoricalDAO::get(DeleteList& del,
                                     const VendorCode& vendor,
                                     int itemNo,
                                     const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  SvcFeesAccountCodeHistoricalKey key(vendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<SvcFeesAccCodeInfo*>* ret = new std::vector<SvcFeesAccCodeInfo*>;
  del.adopt(ret);
  remove_copy_if(
      ptr->begin(), ptr->end(), back_inserter(*ret), IsNotCurrentH<SvcFeesAccCodeInfo>(ticketDate));
  return *ret;
}

std::vector<SvcFeesAccCodeInfo*>*
SvcFeesAccountCodeHistoricalDAO::create(SvcFeesAccountCodeHistoricalKey key)
{
  std::vector<SvcFeesAccCodeInfo*>* ret = new std::vector<SvcFeesAccCodeInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetSvcFeesAccountCodeHistorical fnc(dbAdapter->getAdapter());
    fnc.findSvcFeesAccCodeInfo(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in SvcFeesAccountCodeHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
SvcFeesAccountCodeHistoricalDAO::destroy(SvcFeesAccountCodeHistoricalKey key,
                                         std::vector<SvcFeesAccCodeInfo*>* recs)
{
  std::vector<SvcFeesAccCodeInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
SvcFeesAccountCodeHistoricalDAO::_name("SvcFeesAccountCodeHistorical");
std::string
SvcFeesAccountCodeHistoricalDAO::_cacheClass("Rules");
DAOHelper<SvcFeesAccountCodeHistoricalDAO>
SvcFeesAccountCodeHistoricalDAO::_helper(_name);
SvcFeesAccountCodeHistoricalDAO* SvcFeesAccountCodeHistoricalDAO::_instance = nullptr;

} // namespace tse
