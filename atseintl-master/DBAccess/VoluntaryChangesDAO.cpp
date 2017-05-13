//-----------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly
// prohibited.
//-----------------------------------------------------------------------------
#include "DBAccess/VoluntaryChangesDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetVoluntaryChangesInfo.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"
#include "DBAccess/VoluntaryChangesInfo.h"

namespace tse
{
log4cxx::LoggerPtr
VoluntaryChangesDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.VoluntaryChangesDAO"));
VoluntaryChangesDAO&
VoluntaryChangesDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const VoluntaryChangesInfo*
getVoluntaryChangesData(const VendorCode& vendor,
                        int itemNo,
                        DeleteList& deleteList,
                        const DateTime& ticketDate,
                        bool isHistorical,
                        const DateTime& applDate)
{
  if (isHistorical)
  {
    VoluntaryChangesHistoricalDAO& dao = VoluntaryChangesHistoricalDAO::instance();
    if (applDate != DateTime::emptyDate())
    {
      const VoluntaryChangesInfo* vci = dao.get(deleteList, vendor, itemNo, applDate);
      if (vci)
        return vci;
    }
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    VoluntaryChangesDAO& dao = VoluntaryChangesDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const VoluntaryChangesInfo*
VoluntaryChangesDAO::get(DeleteList& del,
                         const VendorCode& vendor,
                         int itemNo,
                         const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  VoluntaryChangesKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentG<VoluntaryChangesInfo> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter) && NotInhibit<VoluntaryChangesInfo>(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<VoluntaryChangesInfo*>*
VoluntaryChangesDAO::create(VoluntaryChangesKey key)
{
  std::vector<VoluntaryChangesInfo*>* ret = new std::vector<VoluntaryChangesInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetVoluntaryChangesInfo vci(dbAdapter->getAdapter());
    vci.findVoluntaryChangesInfo(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in VoluntaryChangesDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
VoluntaryChangesDAO::destroy(VoluntaryChangesKey key, std::vector<VoluntaryChangesInfo*>* recs)
{
  std::vector<VoluntaryChangesInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
VoluntaryChangesDAO::compress(const std::vector<VoluntaryChangesInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<VoluntaryChangesInfo*>*
VoluntaryChangesDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<VoluntaryChangesInfo>(compressed);
}

std::string
VoluntaryChangesDAO::_name("VoluntaryChanges");
std::string
VoluntaryChangesDAO::_cacheClass("Rules");
DAOHelper<VoluntaryChangesDAO>
VoluntaryChangesDAO::_helper(_name);
VoluntaryChangesDAO* VoluntaryChangesDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: VoluntaryChangesHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
VoluntaryChangesHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.VoluntaryChangesHistoricalDAO"));
VoluntaryChangesHistoricalDAO&
VoluntaryChangesHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const VoluntaryChangesInfo*
VoluntaryChangesHistoricalDAO::get(DeleteList& del,
                                   const VendorCode& vendor,
                                   int itemNo,
                                   const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  VoluntaryChangesHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentH<VoluntaryChangesInfo> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter) && NotInhibit<VoluntaryChangesInfo>(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<VoluntaryChangesInfo*>*
VoluntaryChangesHistoricalDAO::create(VoluntaryChangesHistoricalKey key)
{
  std::vector<VoluntaryChangesInfo*>* ret = new std::vector<VoluntaryChangesInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetVoluntaryChangesInfoHistorical vci(dbAdapter->getAdapter());
    vci.findVoluntaryChangesInfo(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in VoluntaryChangesHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
VoluntaryChangesHistoricalDAO::destroy(VoluntaryChangesHistoricalKey key,
                                       std::vector<VoluntaryChangesInfo*>* recs)
{
  std::vector<VoluntaryChangesInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
VoluntaryChangesHistoricalDAO::compress(const std::vector<VoluntaryChangesInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<VoluntaryChangesInfo*>*
VoluntaryChangesHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<VoluntaryChangesInfo>(compressed);
}

std::string
VoluntaryChangesHistoricalDAO::_name("VoluntaryChangesHistorical");
std::string
VoluntaryChangesHistoricalDAO::_cacheClass("Rules");
DAOHelper<VoluntaryChangesHistoricalDAO>
VoluntaryChangesHistoricalDAO::_helper(_name);
VoluntaryChangesHistoricalDAO* VoluntaryChangesHistoricalDAO::_instance = nullptr;

} // namespace tse
