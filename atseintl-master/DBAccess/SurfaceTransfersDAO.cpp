//-----------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly
// prohibited.
//-----------------------------------------------------------------------------
#include "DBAccess/SurfaceTransfersDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetSurfaceTransfersInfo.h"
#include "DBAccess/SurfaceTransfersInfo.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

namespace tse
{
log4cxx::LoggerPtr
SurfaceTransfersDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SurfaceTransfersDAO"));
SurfaceTransfersDAO&
SurfaceTransfersDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<SurfaceTransfersInfo*>&
getSurfaceTransfersData(const VendorCode& vendor,
                        int itemNo,
                        DeleteList& deleteList,
                        const DateTime& ticketDate,
                        bool isHistorical)
{
  if (isHistorical)
  {
    SurfaceTransfersHistoricalDAO& dao = SurfaceTransfersHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    SurfaceTransfersDAO& dao = SurfaceTransfersDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const std::vector<SurfaceTransfersInfo*>&
SurfaceTransfersDAO::get(DeleteList& del,
                         const VendorCode& vendor,
                         int itemNo,
                         const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  SurfaceTransfersKey key(vendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  std::vector<SurfaceTransfersInfo*>* ret = new std::vector<SurfaceTransfersInfo*>;
  del.adopt(ret);

  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotCurrentG<SurfaceTransfersInfo>(ticketDate));

  ret->erase(std::remove_if(ret->begin(), ret->end(), Inhibit<SurfaceTransfersInfo>), ret->end());

  return *ret;
}

std::vector<SurfaceTransfersInfo*>*
SurfaceTransfersDAO::create(SurfaceTransfersKey key)
{
  std::vector<SurfaceTransfersInfo*>* ret = new std::vector<SurfaceTransfersInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetSurfaceTransfersInfo sti(dbAdapter->getAdapter());
    sti.findSurfaceTransfersInfo(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in SurfaceTransfersDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
SurfaceTransfersDAO::destroy(SurfaceTransfersKey key, std::vector<SurfaceTransfersInfo*>* recs)
{
  std::vector<SurfaceTransfersInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
SurfaceTransfersDAO::_name("SurfaceTransfers");
std::string
SurfaceTransfersDAO::_cacheClass("Rules");
DAOHelper<SurfaceTransfersDAO>
SurfaceTransfersDAO::_helper(_name);
SurfaceTransfersDAO* SurfaceTransfersDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: SurfaceTransfersHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
SurfaceTransfersHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SurfaceTransfersHistoricalDAO"));
SurfaceTransfersHistoricalDAO&
SurfaceTransfersHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<SurfaceTransfersInfo*>&
SurfaceTransfersHistoricalDAO::get(DeleteList& del,
                                   const VendorCode& vendor,
                                   int itemNo,
                                   const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  SurfaceTransfersHistoricalKey key(vendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  std::vector<SurfaceTransfersInfo*>* ret = new std::vector<SurfaceTransfersInfo*>;
  del.adopt(ret);

  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotCurrentH<SurfaceTransfersInfo>(ticketDate));

  ret->erase(std::remove_if(ret->begin(), ret->end(), Inhibit<SurfaceTransfersInfo>), ret->end());

  return *ret;
}

std::vector<SurfaceTransfersInfo*>*
SurfaceTransfersHistoricalDAO::create(SurfaceTransfersHistoricalKey key)
{
  std::vector<SurfaceTransfersInfo*>* ret = new std::vector<SurfaceTransfersInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetSurfaceTransfersInfoHistorical sti(dbAdapter->getAdapter());
    sti.findSurfaceTransfersInfo(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in SurfaceTransfersHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
SurfaceTransfersHistoricalDAO::destroy(SurfaceTransfersHistoricalKey key,
                                       std::vector<SurfaceTransfersInfo*>* recs)
{
  std::vector<SurfaceTransfersInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
SurfaceTransfersHistoricalDAO::_name("SurfaceTransfersHistorical");
std::string
SurfaceTransfersHistoricalDAO::_cacheClass("Rules");
DAOHelper<SurfaceTransfersHistoricalDAO>
SurfaceTransfersHistoricalDAO::_helper(_name);
SurfaceTransfersHistoricalDAO* SurfaceTransfersHistoricalDAO::_instance = nullptr;

} // namespace tse
