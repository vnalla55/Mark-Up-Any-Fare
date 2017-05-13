//-------------------------------------------------------------------------------
// Copyright 2013, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/SectorDetailDAO.h"

#include "Common/Logger.h"
#include "Common/Vendor.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetSectorDetail.h"
#include "DBAccess/SectorDetailInfo.h"

namespace tse
{
log4cxx::LoggerPtr
SectorDetailDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SectorDetailDAO"));

SectorDetailDAO&
SectorDetailDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

bool
SectorDetailDAO::translateKey(const ObjectKey& objectKey, SectorDetailKey& key) const
{
  return key.initialized =
             objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
}

void
SectorDetailDAO::translateKey(const SectorDetailKey& key, ObjectKey& objectKey) const
{
  objectKey.setValue("VENDOR", key._a);
  objectKey.setValue("ITEMNO", key._b);
}

bool
SectorDetailDAO::insertDummyObject(std::string& flatKey, ObjectKey& objectKey)
{
  return DummyObjectInserter<SectorDetailInfo, SectorDetailDAO>(flatKey, objectKey).success();
}

SectorDetailDAO::SectorDetailDAO(int cacheSize /* default = 0 */,
                                 const std::string& cacheType /* default = "" */)
  : DataAccessObject<SectorDetailKey, std::vector<const SectorDetailInfo*> >(
        cacheSize, cacheType, 3)
{
}

const std::vector<const SectorDetailInfo*>
getSectorDetailData(const VendorCode& vendor,
                    int itemNo,
                    DeleteList& deleteList,
                    const DateTime& ticketDate,
                    bool isHistorical)
{
  if (isHistorical)
  {
    return SectorDetailHistoricalDAO::instance().get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    return SectorDetailDAO::instance().get(deleteList, vendor, itemNo, ticketDate);
  }
}

const std::vector<const SectorDetailInfo*>
SectorDetailDAO::get(DeleteList& del,
                     const VendorCode& vendor,
                     int itemNo,
                     const DateTime& ticketDate)
{
  _codeCoverageGetCallCount++;
  std::vector<const SectorDetailInfo*> resultVector;
  SectorDetailKey key(vendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  if (!ptr->empty())
  {
    del.copy(ptr);
    IsCurrentG<SectorDetailInfo> isCurrent(ticketDate);
    for (DAOCache::value_type::const_iterator iter = ptr->begin(); iter != ptr->end(); ++iter)
    {
      if (isCurrent(*iter))
        resultVector.push_back(*iter);
    }
  }
  return resultVector;
}

std::vector<const SectorDetailInfo*>*
SectorDetailDAO::create(const SectorDetailKey key)
{
  std::vector<const SectorDetailInfo*>* ret = new std::vector<const SectorDetailInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetSectorDetail bo(dbAdapter->getAdapter());
    bo.findSectorDetailInfo(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in SectorDetailDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
SectorDetailDAO::destroy(const SectorDetailKey key, std::vector<const SectorDetailInfo*>* recs)
{
  destroyContainer(recs);
}

const SectorDetailKey
SectorDetailDAO::createKey(const SectorDetailInfo* info)
{
  return SectorDetailKey(info->vendor(), info->itemNo());
}

void
SectorDetailDAO::load()
{
  StartupLoaderNoDB<SectorDetailInfo, SectorDetailDAO>();
}

const std::string
SectorDetailDAO::_name("SectorDetail");
const std::string
SectorDetailDAO::_cacheClass("Taxes");
DAOHelper<SectorDetailDAO>
SectorDetailDAO::_helper(_name);
SectorDetailDAO* SectorDetailDAO::_instance = nullptr;

log4cxx::LoggerPtr
SectorDetailHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SectorDetailHistoricalDAO"));

SectorDetailHistoricalDAO&
SectorDetailHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<const SectorDetailInfo*>
SectorDetailHistoricalDAO::get(DeleteList& del,
                               const VendorCode& vendor,
                               int itemNo,
                               const DateTime& ticketDate)
{
  _codeCoverageGetCallCount++;
  std::vector<const SectorDetailInfo*> resultVector;
  SectorDetailHistoricalKey key(vendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  if (!ptr->empty())
  {
    del.copy(ptr);
    IsCurrentH<SectorDetailInfo> isCurrent(ticketDate);
    for (DAOCache::value_type::const_iterator iter = ptr->begin(); iter != ptr->end(); ++iter)
    {
      if (isCurrent(*iter))
        resultVector.push_back(*iter);
    }
  }
  return resultVector;
}

std::vector<const SectorDetailInfo*>*
SectorDetailHistoricalDAO::create(const SectorDetailHistoricalKey key)
{
  std::vector<const SectorDetailInfo*>* ret = new std::vector<const SectorDetailInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetSectorDetailHistorical bo(dbAdapter->getAdapter());
    bo.findSectorDetailInfo(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in SectorDetailHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
SectorDetailHistoricalDAO::destroy(const SectorDetailHistoricalKey key,
                                   std::vector<const SectorDetailInfo*>* recs)
{
  destroyContainer(recs);
}

bool
SectorDetailHistoricalDAO::translateKey(const ObjectKey& objectKey, SectorDetailHistoricalKey& key)
    const
{
  return key.initialized =
             objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b) &&
             objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
}

bool
SectorDetailHistoricalDAO::translateKey(const ObjectKey& objectKey,
                                        SectorDetailHistoricalKey& key,
                                        const DateTime ticketDate) const
{
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  return key.initialized =
             objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
}

void
SectorDetailHistoricalDAO::setKeyDateRange(SectorDetailHistoricalKey& key,
                                           const DateTime ticketDate) const
{
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
}

SectorDetailHistoricalDAO::SectorDetailHistoricalDAO(int cacheSize, /*default = 0*/
                                                     const std::string& cacheType /*default = "" */)
  : HistoricalDataAccessObject<SectorDetailHistoricalKey, std::vector<const SectorDetailInfo*> >(
        cacheSize, cacheType, 3)
{
}

const std::string
SectorDetailHistoricalDAO::_name("SectorDetailHistorical");
const std::string
SectorDetailHistoricalDAO::_cacheClass("Taxes");
DAOHelper<SectorDetailHistoricalDAO>
SectorDetailHistoricalDAO::_helper(_name);
SectorDetailHistoricalDAO* SectorDetailHistoricalDAO::_instance = nullptr;

} // namespace tse
