//-------------------------------------------------------------------------------
// Copyright 2013, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DBAccess/SvcFeesFareIdDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetSvcFeesFareId.h"
#include "DBAccess/SvcFeesFareIdInfo.h"

namespace tse
{
log4cxx::LoggerPtr
SvcFeesFareIdDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SvcFeesFareIdDAO"));

SvcFeesFareIdDAO&
SvcFeesFareIdDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<SvcFeesFareIdInfo*>&
getSvcFeesFareIdData(const VendorCode& vendor,
                     long long itemNo,
                     DeleteList& deleteList,
                     const DateTime& ticketDate,
                     bool isHistorical)
{
  if (isHistorical)
  {
    SvcFeesFareIdHistoricalDAO& dao(SvcFeesFareIdHistoricalDAO::instance());

    const std::vector<SvcFeesFareIdInfo*>& ret(dao.get(deleteList, vendor, itemNo, ticketDate));

    return ret;
  }
  else
  {
    SvcFeesFareIdDAO& dao(SvcFeesFareIdDAO::instance());

    const std::vector<SvcFeesFareIdInfo*>& ret(dao.get(deleteList, vendor, itemNo, ticketDate));

    return ret;
  }
}

const std::vector<SvcFeesFareIdInfo*>&
SvcFeesFareIdDAO::get(DeleteList& del,
                      const VendorCode& vendor,
                      long long itemNo,
                      const DateTime& ticketDate)
{
  // Track calls for code coverage
  ++_codeCoverageGetCallCount;

  SvcFeesFareIdKey key(vendor, itemNo);
  DAOCache::pointer_type ptr(cache().get(key));
  IsNotCurrentG<SvcFeesFareIdInfo> isNotCurrent(ticketDate);
  return *(applyFilter(del, ptr, isNotCurrent));
}

std::vector<SvcFeesFareIdInfo*>*
SvcFeesFareIdDAO::create(SvcFeesFareIdKey key)
{
  std::vector<SvcFeesFareIdInfo*>* ret(new std::vector<SvcFeesFareIdInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get(this->cacheClass()));
  try
  {
    QueryGetSvcFeesFareId q(dbAdapter->getAdapter());
    q.findSvcFeesFareId(*ret, key._a, key._b);
    /*
    std::cerr << "  !!!!!!!!!QueryGetSvcFeesFareId: " << ' ' << ret->size() << " !!!!!!!!!!" <<
    std::endl;

    std::vector<SvcFeesFareIdInfo*> *all(new std::vector<SvcFeesFareIdInfo*>);// test
    QueryGetAllSvcFeesFareId qall(dbAdapter->getAdapter());// test
    qall.findAllSvcFeesFareId(*all);// test
    std::cerr << "  !!!!!!! QueryGetAllSvcFeesFareId: " << ' ' << all->size() << " !!!!!!" <<
    std::endl;
    */
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in SvcFeesFareIdDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

SvcFeesFareIdKey
SvcFeesFareIdDAO::createKey(const SvcFeesFareIdInfo* info)
{
  return SvcFeesFareIdKey(info->vendor(), info->itemNo());
}

void
SvcFeesFareIdDAO::destroy(SvcFeesFareIdKey, std::vector<SvcFeesFareIdInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
SvcFeesFareIdDAO::_name("SvcFeesFareId");
std::string
SvcFeesFareIdDAO::_cacheClass("Fares");
DAOHelper<SvcFeesFareIdDAO>
SvcFeesFareIdDAO::_helper(_name);
SvcFeesFareIdDAO*
SvcFeesFareIdDAO::_instance(nullptr);

// Historical DAO Object
log4cxx::LoggerPtr
SvcFeesFareIdHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SvcFeesFareIdHistoricalDAO"));

SvcFeesFareIdHistoricalDAO&
SvcFeesFareIdHistoricalDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<SvcFeesFareIdInfo*>&
SvcFeesFareIdHistoricalDAO::get(DeleteList& del,
                                const VendorCode& vendor,
                                long long itemNo,
                                const DateTime& ticketDate)
{
  // Track calls for code coverage
  ++_codeCoverageGetCallCount;

  SvcFeesFareIdHistoricalKey key(vendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr(cache().get(key));
  del.copy(ptr);
  std::vector<SvcFeesFareIdInfo*>* ret(new std::vector<SvcFeesFareIdInfo*>);
  del.adopt(ret);
  IsNotCurrentH<SvcFeesFareIdInfo> isNotCurrent(ticketDate);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), isNotCurrent);
  return *ret;
}

std::vector<SvcFeesFareIdInfo*>*
SvcFeesFareIdHistoricalDAO::create(SvcFeesFareIdHistoricalKey key)
{
  std::vector<SvcFeesFareIdInfo*>* ret(new std::vector<SvcFeesFareIdInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get("Historical", true));
  try
  {
    QueryGetSvcFeesFareIdHistorical q(dbAdapter->getAdapter());
    q.findSvcFeesFareId(*ret, key._a, key._b, key._c, key._d);
    // std::cerr << "  !!!! QueryGetSvcFeesFareIdHistorical: " << ' ' << ret->size() << std::endl;
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in SvcFeesFareIdHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

SvcFeesFareIdHistoricalKey
SvcFeesFareIdHistoricalDAO::createKey(const SvcFeesFareIdInfo* info,
                                      const DateTime& startDate,
                                      const DateTime& endDate)
{
  return SvcFeesFareIdHistoricalKey(info->vendor(), info->itemNo(), startDate, endDate);
}

void
SvcFeesFareIdHistoricalDAO::destroy(SvcFeesFareIdHistoricalKey,
                                    std::vector<SvcFeesFareIdInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
SvcFeesFareIdHistoricalDAO::_name("SvcFeesFareIdHistorical");
std::string
SvcFeesFareIdHistoricalDAO::_cacheClass("Fares");
DAOHelper<SvcFeesFareIdHistoricalDAO>
SvcFeesFareIdHistoricalDAO::_helper(_name);
SvcFeesFareIdHistoricalDAO*
SvcFeesFareIdHistoricalDAO::_instance(nullptr);

} // namespace tse
