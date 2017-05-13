//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/TpdPsrDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetTpdPsr.h"
#include "DBAccess/TpdPsr.h"

namespace tse
{
log4cxx::LoggerPtr
TpdPsrDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.TpdPsrDAO"));
TpdPsrDAO&
TpdPsrDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<TpdPsr*>&
getTpdPsrData(Indicator applInd,
              const CarrierCode& carrier,
              Indicator area1,
              Indicator area2,
              const DateTime& date,
              DeleteList& deleteList,
              const DateTime& ticketDate,
              bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    TpdPsrHistoricalDAO& dao = TpdPsrHistoricalDAO::instance();
    return dao.get(deleteList, applInd, carrier, area1, area2, date, ticketDate);
  }
  else
  {
    TpdPsrDAO& dao = TpdPsrDAO::instance();
    return dao.get(deleteList, applInd, carrier, area1, area2, date, ticketDate);
  }
}

const std::vector<TpdPsr*>&
TpdPsrDAO::get(DeleteList& del,
               Indicator applInd,
               const CarrierCode& carrier,
               Indicator area1,
               Indicator area2,
               const DateTime& date,
               const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  if (area1 > area2)
  {
    // swap the areas if in wrong order
    Indicator temp = area1;
    area1 = area2;
    area2 = temp;
  }

  TpdPsrKey key(applInd, carrier, area1, area2);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<TpdPsr*>* ret = new std::vector<TpdPsr*>;
  remove_copy_if(
      ptr->begin(), ptr->end(), back_inserter(*ret), IsNotEffectiveG<TpdPsr>(date, ticketDate));
  if (!carrier.empty())
  {
    TpdPsrKey key2(applInd, "", area1, area2);
    DAOCache::pointer_type ptr = cache().get(key2);
    del.copy(ptr);
    remove_copy_if(
        ptr->begin(), ptr->end(), back_inserter(*ret), IsNotEffectiveG<TpdPsr>(date, ticketDate));
  }

  del.adopt(ret);
  return *ret;
}

TpdPsrKey
TpdPsrDAO::createKey(TpdPsr* info)
{
  return TpdPsrKey(info->applInd(), info->carrier(), info->area1(), info->area2());
}

std::vector<TpdPsr*>*
TpdPsrDAO::create(TpdPsrKey key)
{
  std::vector<TpdPsr*>* ret = new std::vector<TpdPsr*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTpdPsrBase tpb(dbAdapter->getAdapter());
    tpb.findTpdPsr(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TpdPsrDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
TpdPsrDAO::destroy(TpdPsrKey key, std::vector<TpdPsr*>* recs)
{
  destroyContainer(recs);
}

void
TpdPsrDAO::load()
{
  StartupLoader<QueryGetAllTpdPsrBase, TpdPsr, TpdPsrDAO>();
}

sfc::CompressedData*
TpdPsrDAO::compress(const std::vector<TpdPsr*>* vect) const
{
  return compressVector(vect);
}

std::vector<TpdPsr*>*
TpdPsrDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<TpdPsr>(compressed);
}

std::string
TpdPsrDAO::_name("TpdPsr");
std::string
TpdPsrDAO::_cacheClass("Routing");
DAOHelper<TpdPsrDAO>
TpdPsrDAO::_helper(_name);
TpdPsrDAO* TpdPsrDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: TpdPsrHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
TpdPsrHistoricalDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.TpdPsrHistoricalDAO"));
TpdPsrHistoricalDAO&
TpdPsrHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<TpdPsr*>&
TpdPsrHistoricalDAO::get(DeleteList& del,
                         Indicator applInd,
                         const CarrierCode& carrier,
                         Indicator area1,
                         Indicator area2,
                         const DateTime& date,
                         const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  if (area1 > area2)
  {
    // swap the areas if in wrong order
    Indicator temp = area1;
    area1 = area2;
    area2 = temp;
  }

  TpdPsrHistoricalKey key(applInd, carrier, area1, area2);
  DAOUtils::getDateRange(ticketDate, key._e, key._f, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<TpdPsr*>* ret = new std::vector<TpdPsr*>;
  remove_copy_if(
      ptr->begin(), ptr->end(), back_inserter(*ret), IsNotEffectiveHist<TpdPsr>(date, ticketDate));
  if (!carrier.empty())
  {
    TpdPsrHistoricalKey key2(applInd, "", area1, area2);
    DAOUtils::getDateRange(ticketDate, key2._e, key2._f, _cacheBy);
    DAOCache::pointer_type ptr = cache().get(key2);
    del.copy(ptr);
    remove_copy_if(ptr->begin(),
                   ptr->end(),
                   back_inserter(*ret),
                   IsNotEffectiveHist<TpdPsr>(date, ticketDate));
  }

  del.adopt(ret);
  return *ret;
}

std::vector<TpdPsr*>*
TpdPsrHistoricalDAO::create(TpdPsrHistoricalKey key)
{
  std::vector<TpdPsr*>* ret = new std::vector<TpdPsr*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTpdPsrBaseHistorical tpb(dbAdapter->getAdapter());
    tpb.findTpdPsr(*ret, key._a, key._b, key._c, key._d, key._e, key._f);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TpdPsrHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
TpdPsrHistoricalDAO::destroy(TpdPsrHistoricalKey key, std::vector<TpdPsr*>* recs)
{
  destroyContainer(recs);
}

sfc::CompressedData*
TpdPsrHistoricalDAO::compress(const std::vector<TpdPsr*>* vect) const
{
  return compressVector(vect);
}

std::vector<TpdPsr*>*
TpdPsrHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<TpdPsr>(compressed);
}

std::string
TpdPsrHistoricalDAO::_name("TpdPsrHistorical");
std::string
TpdPsrHistoricalDAO::_cacheClass("Routing");
DAOHelper<TpdPsrHistoricalDAO>
TpdPsrHistoricalDAO::_helper(_name);
TpdPsrHistoricalDAO* TpdPsrHistoricalDAO::_instance = nullptr;

} // namespace tse
