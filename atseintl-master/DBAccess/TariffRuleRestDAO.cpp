//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/TariffRuleRestDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetTariffRuleRest.h"
#include "DBAccess/TariffRuleRest.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

using namespace std;

namespace tse
{
log4cxx::LoggerPtr
TariffRuleRestDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.TariffRuleRestDAO"));
TariffRuleRestDAO&
TariffRuleRestDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<TariffRuleRest*>&
getTariffRuleRestData(const VendorCode& vendor,
                      int itemNo,
                      DeleteList& deleteList,
                      const DateTime& ticketDate,
                      bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    TariffRuleRestHistoricalDAO& dao = TariffRuleRestHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    TariffRuleRestDAO& dao = TariffRuleRestDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const std::vector<TariffRuleRest*>&
TariffRuleRestDAO::get(DeleteList& del,
                       const VendorCode& vendor,
                       int itemNo,
                       const DateTime& ticketDate)
{
  // Track calls for code coverage
  ++_codeCoverageGetCallCount;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  TariffRuleRestKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  std::vector<TariffRuleRest*>* ret(new std::vector<TariffRuleRest*>);
  del.adopt(ret);

  remove_copy_if(
      ptr->begin(), ptr->end(), back_inserter(*ret), IsNotCurrentG<TariffRuleRest>(ticketDate));

  return *ret;
}

std::vector<TariffRuleRest*>*
TariffRuleRestDAO::create(TariffRuleRestKey key)
{
  std::vector<TariffRuleRest*>* ret = new std::vector<TariffRuleRest*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTariffRuleRest trr(dbAdapter->getAdapter());
    trr.findTariffRuleRest(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TariffRuleRestDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
TariffRuleRestDAO::destroy(TariffRuleRestKey key, std::vector<TariffRuleRest*>* recs)
{
  destroyContainer(recs);
}

TariffRuleRestKey
TariffRuleRestDAO::createKey(TariffRuleRest* info)
{
  return TariffRuleRestKey(info->vendor(), info->itemNo());
}

void
TariffRuleRestDAO::load()
{
  StartupLoaderNoDB<TariffRuleRest, TariffRuleRestDAO>();
}

sfc::CompressedData*
TariffRuleRestDAO::compress(const std::vector<TariffRuleRest*>* vect) const
{
  return compressVector(vect);
}

std::vector<TariffRuleRest*>*
TariffRuleRestDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<TariffRuleRest>(compressed);
}

std::string
TariffRuleRestDAO::_name("TariffRuleRest");
std::string
TariffRuleRestDAO::_cacheClass("Rules");
DAOHelper<TariffRuleRestDAO>
TariffRuleRestDAO::_helper(_name);
TariffRuleRestDAO* TariffRuleRestDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: TariffRuleRestHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
TariffRuleRestHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.TariffRuleRestHistoricalDAO"));
TariffRuleRestHistoricalDAO&
TariffRuleRestHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<TariffRuleRest*>&
TariffRuleRestHistoricalDAO::get(DeleteList& del,
                                 const VendorCode& vendor,
                                 int itemNo,
                                 const DateTime& ticketDate)
{
  // Track calls for code coverage
  ++_codeCoverageGetCallCount;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  TariffRuleRestHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  std::vector<TariffRuleRest*>* ret(new std::vector<TariffRuleRest*>);
  del.adopt(ret);

  remove_copy_if(
      ptr->begin(), ptr->end(), back_inserter(*ret), IsNotCurrentH<TariffRuleRest>(ticketDate));

  return *ret;
}

std::vector<TariffRuleRest*>*
TariffRuleRestHistoricalDAO::create(TariffRuleRestHistoricalKey key)
{
  std::vector<TariffRuleRest*>* ret = new std::vector<TariffRuleRest*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTariffRuleRestHistorical trr(dbAdapter->getAdapter());
    trr.findTariffRuleRest(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TariffRuleRestHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
TariffRuleRestHistoricalDAO::destroy(TariffRuleRestHistoricalKey key,
                                     std::vector<TariffRuleRest*>* recs)
{
  destroyContainer(recs);
}

sfc::CompressedData*
TariffRuleRestHistoricalDAO::compress(const std::vector<TariffRuleRest*>* vect) const
{
  return compressVector(vect);
}

std::vector<TariffRuleRest*>*
TariffRuleRestHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<TariffRuleRest>(compressed);
}

std::string
TariffRuleRestHistoricalDAO::_name("TariffRuleRestHistorical");
std::string
TariffRuleRestHistoricalDAO::_cacheClass("Rules");
DAOHelper<TariffRuleRestHistoricalDAO>
TariffRuleRestHistoricalDAO::_helper(_name);
TariffRuleRestHistoricalDAO* TariffRuleRestHistoricalDAO::_instance = nullptr;

} // namespace tse
