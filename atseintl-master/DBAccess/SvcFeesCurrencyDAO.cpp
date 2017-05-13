//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/SvcFeesCurrencyDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DAOUtils.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DBHistoryServer.h"
#include "DBAccess/Queries/QueryGetSvcFeesCurrency.h"
#include "DBAccess/SvcFeesCurrencyInfo.h"

namespace tse
{
log4cxx::LoggerPtr
SvcFeesCurrencyDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SvcFeesCurrencyDAO"));

SvcFeesCurrencyDAO&
SvcFeesCurrencyDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<SvcFeesCurrencyInfo*>&
getSvcFeesCurrencyData(const VendorCode& vendor,
                       int itemNo,
                       DeleteList& deleteList,
                       const DateTime& ticketDate,
                       bool isHistorical)
{
  if (isHistorical)
  {
    SvcFeesCurrencyHistoricalDAO& dao = SvcFeesCurrencyHistoricalDAO::instance();

    const std::vector<SvcFeesCurrencyInfo*>& ret = dao.get(deleteList, vendor, itemNo, ticketDate);

    return ret;
  }
  else
  {
    SvcFeesCurrencyDAO& dao = SvcFeesCurrencyDAO::instance();

    const std::vector<SvcFeesCurrencyInfo*>& ret = dao.get(deleteList, vendor, itemNo, ticketDate);

    return ret;
  }
}

const std::vector<SvcFeesCurrencyInfo*>&
SvcFeesCurrencyDAO::get(DeleteList& del,
                        const VendorCode& vendor,
                        int itemNo,
                        const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  SvcFeesCurrencyKey key(vendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<SvcFeesCurrencyInfo*>* ret = new std::vector<SvcFeesCurrencyInfo*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotCurrentG<SvcFeesCurrencyInfo>(ticketDate));
  return *ret;
}

std::vector<SvcFeesCurrencyInfo*>*
SvcFeesCurrencyDAO::create(SvcFeesCurrencyKey key)
{
  std::vector<SvcFeesCurrencyInfo*>* ret = new std::vector<SvcFeesCurrencyInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetSvcFeesCurrency qsfc(dbAdapter->getAdapter());
    qsfc.findSvcFeesCurrencyInfo(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in SvcFeesCurrencyDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
SvcFeesCurrencyDAO::destroy(SvcFeesCurrencyKey key, std::vector<SvcFeesCurrencyInfo*>* recs)
{
  destroyContainer(recs);
}

SvcFeesCurrencyKey
SvcFeesCurrencyDAO::createKey(SvcFeesCurrencyInfo* info)
{
  return SvcFeesCurrencyKey(info->vendor(), info->itemNo());
}

void
SvcFeesCurrencyDAO::load()
{
  // StartupLoader<QueryGetSvcFeesCurrency,SvcFeesCurrencyInfo,SvcFeesCurrencyDAO>() ;
  // Not pre_loading
  StartupLoaderNoDB<SvcFeesCurrencyInfo, SvcFeesCurrencyDAO>();
}

sfc::CompressedData*
SvcFeesCurrencyDAO::compress(const std::vector<SvcFeesCurrencyInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<SvcFeesCurrencyInfo*>*
SvcFeesCurrencyDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<SvcFeesCurrencyInfo>(compressed);
}

std::string
SvcFeesCurrencyDAO::_name("SvcFeesCurrency");
std::string
SvcFeesCurrencyDAO::_cacheClass("Rules");
DAOHelper<SvcFeesCurrencyDAO>
SvcFeesCurrencyDAO::_helper(_name);
SvcFeesCurrencyDAO* SvcFeesCurrencyDAO::_instance = nullptr;

///////////////////////////////////////////////////////
// Historical DAO Object
///////////////////////////////////////////////////////
log4cxx::LoggerPtr
SvcFeesCurrencyHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SvcFeesCurrencyHistDAO"));
SvcFeesCurrencyHistoricalDAO&
SvcFeesCurrencyHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<SvcFeesCurrencyInfo*>&
SvcFeesCurrencyHistoricalDAO::get(DeleteList& del,
                                  const VendorCode& vendor,
                                  int itemNo,
                                  const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  SvcFeesCurrencyHistoricalKey key(vendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<SvcFeesCurrencyInfo*>* ret = new std::vector<SvcFeesCurrencyInfo*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotCurrentH<SvcFeesCurrencyInfo>(ticketDate));
  return *ret;
}

std::vector<SvcFeesCurrencyInfo*>*
SvcFeesCurrencyHistoricalDAO::create(SvcFeesCurrencyHistoricalKey key)
{
  std::vector<SvcFeesCurrencyInfo*>* ret = new std::vector<SvcFeesCurrencyInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetSvcFeesCurrencyHistorical fnc(dbAdapter->getAdapter());
    fnc.findSvcFeesCurrencyInfo(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in SvcFeesCurrencyHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
SvcFeesCurrencyHistoricalDAO::destroy(SvcFeesCurrencyHistoricalKey key,
                                      std::vector<SvcFeesCurrencyInfo*>* recs)
{
  std::vector<SvcFeesCurrencyInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
SvcFeesCurrencyHistoricalDAO::compress(const std::vector<SvcFeesCurrencyInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<SvcFeesCurrencyInfo*>*
SvcFeesCurrencyHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<SvcFeesCurrencyInfo>(compressed);
}

std::string
SvcFeesCurrencyHistoricalDAO::_name("SvcFeesCurrencyHistorical");
std::string
SvcFeesCurrencyHistoricalDAO::_cacheClass("Rules");
DAOHelper<SvcFeesCurrencyHistoricalDAO>
SvcFeesCurrencyHistoricalDAO::_helper(_name);
SvcFeesCurrencyHistoricalDAO* SvcFeesCurrencyHistoricalDAO::_instance = nullptr;

} // namespace tse
