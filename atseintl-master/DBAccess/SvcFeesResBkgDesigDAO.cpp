//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/SvcFeesResBkgDesigDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DAOUtils.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DBHistoryServer.h"
#include "DBAccess/Queries/QueryGetSvcFeesResBkgDesig.h"
#include "DBAccess/SvcFeesResBkgDesigInfo.h"

namespace tse
{
log4cxx::LoggerPtr
SvcFeesResBkgDesigDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SvcFeesResBkgDesigDAO"));

SvcFeesResBkgDesigDAO&
SvcFeesResBkgDesigDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<SvcFeesResBkgDesigInfo*>&
getSvcFeesResBkgDesigData(const VendorCode& vendor,
                          int itemNo,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    SvcFeesResBkgDesigHistoricalDAO& dao = SvcFeesResBkgDesigHistoricalDAO::instance();

    const std::vector<SvcFeesResBkgDesigInfo*>& ret =
        dao.get(deleteList, vendor, itemNo, ticketDate);

    return ret;
  }
  else
  {
    SvcFeesResBkgDesigDAO& dao = SvcFeesResBkgDesigDAO::instance();

    const std::vector<SvcFeesResBkgDesigInfo*>& ret =
        dao.get(deleteList, vendor, itemNo, ticketDate);

    return ret;
  }
}

const std::vector<SvcFeesResBkgDesigInfo*>&
SvcFeesResBkgDesigDAO::get(DeleteList& del,
                           const VendorCode& vendor,
                           int itemNo,
                           const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  SvcFeesResBkgDesigKey key(vendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<SvcFeesResBkgDesigInfo*>* ret = new std::vector<SvcFeesResBkgDesigInfo*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotCurrentG<SvcFeesResBkgDesigInfo>(ticketDate));
  return *ret;
}

std::vector<SvcFeesResBkgDesigInfo*>*
SvcFeesResBkgDesigDAO::create(SvcFeesResBkgDesigKey key)
{
  std::vector<SvcFeesResBkgDesigInfo*>* ret = new std::vector<SvcFeesResBkgDesigInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetSvcFeesResBkgDesig qsfac(dbAdapter->getAdapter());
    qsfac.findSvcFeesResBkgDesigInfo(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in SvcFeesResBkgDesigDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
SvcFeesResBkgDesigDAO::destroy(SvcFeesResBkgDesigKey key,
                               std::vector<SvcFeesResBkgDesigInfo*>* recs)
{
  destroyContainer(recs);
}

SvcFeesResBkgDesigKey
SvcFeesResBkgDesigDAO::createKey(SvcFeesResBkgDesigInfo* info)
{
  return SvcFeesResBkgDesigKey(info->vendor(), info->itemNo());
}

void
SvcFeesResBkgDesigDAO::load()
{
  // StartupLoader<QueryGetSvcFeesResBkgDesig,SvcFeesResBkgDesigInfo,SvcFeesResBkgDesigDAO>() ;
  // Not pre_loading
  StartupLoaderNoDB<SvcFeesResBkgDesigInfo, SvcFeesResBkgDesigDAO>();
}

std::string
SvcFeesResBkgDesigDAO::_name("SvcFeesResBkgDesig");
std::string
SvcFeesResBkgDesigDAO::_cacheClass("Rules");
DAOHelper<SvcFeesResBkgDesigDAO>
SvcFeesResBkgDesigDAO::_helper(_name);
SvcFeesResBkgDesigDAO* SvcFeesResBkgDesigDAO::_instance = nullptr;

///////////////////////////////////////////////////////
// Historical DAO Object
///////////////////////////////////////////////////////
log4cxx::LoggerPtr
SvcFeesResBkgDesigHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SvcFeesResBkgDesigHistDAO"));
SvcFeesResBkgDesigHistoricalDAO&
SvcFeesResBkgDesigHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<SvcFeesResBkgDesigInfo*>&
SvcFeesResBkgDesigHistoricalDAO::get(DeleteList& del,
                                     const VendorCode& vendor,
                                     int itemNo,
                                     const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  SvcFeesResBkgDesigHistoricalKey key(vendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<SvcFeesResBkgDesigInfo*>* ret = new std::vector<SvcFeesResBkgDesigInfo*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotCurrentH<SvcFeesResBkgDesigInfo>(ticketDate));
  return *ret;
}

std::vector<SvcFeesResBkgDesigInfo*>*
SvcFeesResBkgDesigHistoricalDAO::create(SvcFeesResBkgDesigHistoricalKey key)
{
  std::vector<SvcFeesResBkgDesigInfo*>* ret = new std::vector<SvcFeesResBkgDesigInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetSvcFeesResBkgDesigHistorical fnc(dbAdapter->getAdapter());
    fnc.findSvcFeesResBkgDesigInfo(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in SvcFeesResBkgDesigHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
SvcFeesResBkgDesigHistoricalDAO::destroy(SvcFeesResBkgDesigHistoricalKey key,
                                         std::vector<SvcFeesResBkgDesigInfo*>* recs)
{
  std::vector<SvcFeesResBkgDesigInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
SvcFeesResBkgDesigHistoricalDAO::_name("SvcFeesResBkgDesigHistorical");
std::string
SvcFeesResBkgDesigHistoricalDAO::_cacheClass("Rules");
DAOHelper<SvcFeesResBkgDesigHistoricalDAO>
SvcFeesResBkgDesigHistoricalDAO::_helper(_name);
SvcFeesResBkgDesigHistoricalDAO* SvcFeesResBkgDesigHistoricalDAO::_instance = nullptr;

} // namespace tse
