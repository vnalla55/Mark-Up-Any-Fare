//-----------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly
// prohibited.
//-----------------------------------------------------------------------------
#include "DBAccess/Transfers1DAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetTransfersInfo1.h"
#include "DBAccess/TransfersInfo1.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

namespace tse
{
log4cxx::LoggerPtr
Transfers1DAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.Transfers1DAO"));
Transfers1DAO&
Transfers1DAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const TransfersInfo1*
getTransfers1Data(const VendorCode& vendor,
                  int itemNo,
                  DeleteList& deleteList,
                  const DateTime& ticketDate,
                  bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    Transfers1HistoricalDAO& dao = Transfers1HistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    Transfers1DAO& dao = Transfers1DAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const TransfersInfo1*
Transfers1DAO::get(DeleteList& del,
                   const VendorCode& vendor,
                   int itemNo,
                   const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  TransfersKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  if (UNLIKELY(ptr->empty()))
    return nullptr;

  del.copy(ptr);

  IsCurrentG<TransfersInfo1> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (LIKELY(isCurrent(*iter) && NotInhibit<TransfersInfo1>(*iter)))
      return *iter;
  }
  return nullptr;
}

std::vector<TransfersInfo1*>*
Transfers1DAO::create(TransfersKey key)
{
  std::vector<TransfersInfo1*>* ret = new std::vector<TransfersInfo1*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTransfersInfo1 ti1(dbAdapter->getAdapter());
    ti1.findTransfersInfo1(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in Transfers1DAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
Transfers1DAO::destroy(TransfersKey key, std::vector<TransfersInfo1*>* recs)
{
  std::vector<TransfersInfo1*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

TransfersKey
Transfers1DAO::createKey(TransfersInfo1* info)
{
  return TransfersKey(info->vendor(), info->itemNo());
}

void
Transfers1DAO::load()
{
  StartupLoaderNoDB<TransfersInfo1, Transfers1DAO>();
}

sfc::CompressedData*
Transfers1DAO::compress(const std::vector<TransfersInfo1*>* vect) const
{
  return compressVector(vect);
}

std::vector<TransfersInfo1*>*
Transfers1DAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<TransfersInfo1>(compressed);
}

std::string
Transfers1DAO::_name("Transfers1");
std::string
Transfers1DAO::_cacheClass("Rules");
DAOHelper<Transfers1DAO>
Transfers1DAO::_helper(_name);
Transfers1DAO* Transfers1DAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: Transfers1HistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
Transfers1HistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.Transfers1HistoricalDAO"));
Transfers1HistoricalDAO&
Transfers1HistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const TransfersInfo1*
Transfers1HistoricalDAO::get(DeleteList& del,
                             const VendorCode& vendor,
                             int itemNo,
                             const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  TransfersHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentH<TransfersInfo1> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter) && NotInhibit<TransfersInfo1>(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<TransfersInfo1*>*
Transfers1HistoricalDAO::create(TransfersHistoricalKey key)
{
  std::vector<TransfersInfo1*>* ret = new std::vector<TransfersInfo1*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTransfersInfo1Historical ti1(dbAdapter->getAdapter());
    ti1.findTransfersInfo1(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in Transfers1HistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
Transfers1HistoricalDAO::destroy(TransfersHistoricalKey key, std::vector<TransfersInfo1*>* recs)
{
  std::vector<TransfersInfo1*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
Transfers1HistoricalDAO::compress(const std::vector<TransfersInfo1*>* vect) const
{
  return compressVector(vect);
}

std::vector<TransfersInfo1*>*
Transfers1HistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<TransfersInfo1>(compressed);
}

std::string
Transfers1HistoricalDAO::_name("Transfers1Historical");
std::string
Transfers1HistoricalDAO::_cacheClass("Rules");
DAOHelper<Transfers1HistoricalDAO>
Transfers1HistoricalDAO::_helper(_name);
Transfers1HistoricalDAO* Transfers1HistoricalDAO::_instance = nullptr;

} // namespace tse
