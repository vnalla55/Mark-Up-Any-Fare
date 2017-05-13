//-------------------------------------------------------------------------------
// Copyright 2013, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DBAccess/SvcFeesFeatureDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetSvcFeesFeature.h"
#include "DBAccess/SvcFeesFeatureInfo.h"

namespace tse
{
log4cxx::LoggerPtr
SvcFeesFeatureDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SvcFeesFeatureDAO"));

SvcFeesFeatureDAO&
SvcFeesFeatureDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<SvcFeesFeatureInfo*>&
getSvcFeesFeatureData(const VendorCode& vendor,
                      long long itemNo,
                      DeleteList& deleteList,
                      const DateTime& ticketDate,
                      bool isHistorical)
{
  if (isHistorical)
  {
    SvcFeesFeatureHistoricalDAO& dao(SvcFeesFeatureHistoricalDAO::instance());

    const std::vector<SvcFeesFeatureInfo*>& ret(dao.get(deleteList, vendor, itemNo, ticketDate));

    return ret;
  }
  else
  {
    SvcFeesFeatureDAO& dao(SvcFeesFeatureDAO::instance());

    const std::vector<SvcFeesFeatureInfo*>& ret(dao.get(deleteList, vendor, itemNo, ticketDate));

    return ret;
  }
}

const std::vector<SvcFeesFeatureInfo*>&
SvcFeesFeatureDAO::get(DeleteList& del,
                       const VendorCode& vendor,
                       long long itemNo,
                       const DateTime& ticketDate)
{
  // Track calls for code coverage
  ++_codeCoverageGetCallCount;

  SvcFeesFeatureKey key(vendor, itemNo);
  DAOCache::pointer_type ptr(cache().get(key));
  IsNotCurrentG<SvcFeesFeatureInfo> isNotCurrent(ticketDate);
  return *(applyFilter(del, ptr, isNotCurrent));
}

std::vector<SvcFeesFeatureInfo*>*
SvcFeesFeatureDAO::create(SvcFeesFeatureKey key)
{
  std::vector<SvcFeesFeatureInfo*>* ret(new std::vector<SvcFeesFeatureInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get(this->cacheClass()));
  try
  {
    QueryGetSvcFeesFeature q(dbAdapter->getAdapter());
    q.findSvcFeesFeature(*ret, key._a, key._b);
    /*
    std::cerr << "  !!!!!!!!!QueryGetSvcFeesFeature: " << ' ' << ret->size() << " !!!!!!!!!!" <<
    std::endl;

    std::vector<SvcFeesFeatureInfo*> *all(new std::vector<SvcFeesFeatureInfo*>);// test
    QueryGetAllSvcFeesFeature qall(dbAdapter->getAdapter());// test
    qall.findAllSvcFeesFeature(*all);// test
    std::cerr << "  !!!!!!! QueryGetAllSvcFeesFeature: " << ' ' << all->size() << " !!!!!!" <<
    std::endl;
    */
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in SvcFeesFeatureDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

SvcFeesFeatureKey
SvcFeesFeatureDAO::createKey(const SvcFeesFeatureInfo* info)
{
  return SvcFeesFeatureKey(info->vendor(), info->itemNo());
}

void
SvcFeesFeatureDAO::destroy(SvcFeesFeatureKey, std::vector<SvcFeesFeatureInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
SvcFeesFeatureDAO::_name("SvcFeesFeature");
std::string
SvcFeesFeatureDAO::_cacheClass("Fares");
DAOHelper<SvcFeesFeatureDAO>
SvcFeesFeatureDAO::_helper(_name);
SvcFeesFeatureDAO*
SvcFeesFeatureDAO::_instance(nullptr);

// Historical DAO Object
log4cxx::LoggerPtr
SvcFeesFeatureHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SvcFeesFeatureHistoricalDAO"));

SvcFeesFeatureHistoricalDAO&
SvcFeesFeatureHistoricalDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<SvcFeesFeatureInfo*>&
SvcFeesFeatureHistoricalDAO::get(DeleteList& del,
                                 const VendorCode& vendor,
                                 long long itemNo,
                                 const DateTime& ticketDate)
{
  // Track calls for code coverage
  ++_codeCoverageGetCallCount;

  SvcFeesFeatureHistoricalKey key(vendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr(cache().get(key));
  del.copy(ptr);
  std::vector<SvcFeesFeatureInfo*>* ret(new std::vector<SvcFeesFeatureInfo*>);
  del.adopt(ret);
  IsNotCurrentH<SvcFeesFeatureInfo> isNotCurrent(ticketDate);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), isNotCurrent);
  return *ret;
}

std::vector<SvcFeesFeatureInfo*>*
SvcFeesFeatureHistoricalDAO::create(SvcFeesFeatureHistoricalKey key)
{
  std::vector<SvcFeesFeatureInfo*>* ret(new std::vector<SvcFeesFeatureInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get("Historical", true));
  try
  {
    QueryGetSvcFeesFeatureHistorical q(dbAdapter->getAdapter());
    q.findSvcFeesFeature(*ret, key._a, key._b, key._c, key._d);
    std::cerr << "  !!!! QueryGetSvcFeesFeatureHistorical: " << ' ' << ret->size() << std::endl;
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in SvcFeesFeatureHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

SvcFeesFeatureHistoricalKey
SvcFeesFeatureHistoricalDAO::createKey(const SvcFeesFeatureInfo* info,
                                       const DateTime& startDate,
                                       const DateTime& endDate)
{
  return SvcFeesFeatureHistoricalKey(info->vendor(), info->itemNo(), startDate, endDate);
}

void
SvcFeesFeatureHistoricalDAO::destroy(SvcFeesFeatureHistoricalKey,
                                     std::vector<SvcFeesFeatureInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
SvcFeesFeatureHistoricalDAO::_name("SvcFeesFeatureHistorical");
std::string
SvcFeesFeatureHistoricalDAO::_cacheClass("Fares");
DAOHelper<SvcFeesFeatureHistoricalDAO>
SvcFeesFeatureHistoricalDAO::_helper(_name);
SvcFeesFeatureHistoricalDAO*
SvcFeesFeatureHistoricalDAO::_instance(nullptr);

} // namespace tse
