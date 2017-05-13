//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/MinFareApplDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/MinFareAppl.h"
#include "DBAccess/Queries/QueryGetMinFareAppl.h"

namespace tse
{
log4cxx::LoggerPtr
MinFareApplDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.MinFareApplDAO"));

MinFareApplDAO&
MinFareApplDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<MinFareAppl*>&
getMinFareApplData(const VendorCode& textTblVendor,
                   int textTblItemNo,
                   const CarrierCode& governingCarrier,
                   const DateTime& date,
                   DeleteList& deleteList,
                   const DateTime& ticketDate,
                   bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    MinFareApplHistoricalDAO& dao = MinFareApplHistoricalDAO::instance();
    return dao.get(deleteList, textTblVendor, textTblItemNo, governingCarrier, date, ticketDate);
  }
  else
  {
    MinFareApplDAO& dao = MinFareApplDAO::instance();
    return dao.get(deleteList, textTblVendor, textTblItemNo, governingCarrier, date, ticketDate);
  }
}

const std::vector<MinFareAppl*>&
MinFareApplDAO::get(DeleteList& del,
                    const VendorCode& textTblVendor,
                    int textTblItemNo,
                    const CarrierCode& governingCarrier,
                    const DateTime& date,
                    const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  MinFareApplKey key(textTblVendor, textTblItemNo, governingCarrier);
  DAOCache::pointer_type ptr = cache().get(key);
  /*
  del.copy(ptr);
  std::vector<MinFareAppl*>* ret = new std::vector<MinFareAppl*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),ptr->end(),back_inserter(*ret),IsNotEffectiveG<MinFareAppl>(date,ticketDate));
  return *ret;
  */
  return *(applyFilter(del, ptr, IsNotEffectiveG<MinFareAppl>(date, ticketDate)));
}

std::vector<MinFareAppl*>*
MinFareApplDAO::create(MinFareApplKey key)
{
  std::vector<MinFareAppl*>* ret = new std::vector<MinFareAppl*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetMinFareApplBase mfab(dbAdapter->getAdapter());
    mfab.findMinFareAppl(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in MinFareApplDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
MinFareApplDAO::destroy(MinFareApplKey key, std::vector<MinFareAppl*>* recs)
{
  std::vector<MinFareAppl*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

size_t
MinFareApplDAO::invalidate(const ObjectKey& objectKey)
{
  MinFareApplKey invKey;
  if (!translateKey(objectKey, invKey))
  {
    LOG4CXX_ERROR(_logger, "Cache notification key translation error");
    return 0;
  }

  // If the query OR condition default values are not in the key, no overriding
  //_a = texttblvendor; _b = texttblitemno; _c = governingcarrier
  if ((invKey._a != "") && (invKey._b != 0) && (invKey._c != ""))
  {
    return DataAccessObject<MinFareApplKey, std::vector<MinFareAppl*> >::invalidate(invKey);
  }

  // Flush all possible MINFAREAPPL variations of ObjectKey
  std::shared_ptr<std::vector<MinFareApplKey>> cacheKeys = cache().keys();
  std::vector<MinFareApplKey>* pCacheKeys = cacheKeys.get();

  if (pCacheKeys == nullptr)
  {
    LOG4CXX_DEBUG(_logger, "No cache entries to iterate");
    return 0;
  }

  uint32_t nKeys = pCacheKeys->size();
  size_t result(0);
  for (uint32_t i = 0; (i < nKeys); i++)
  {
    MinFareApplKey cacheKey = (*pCacheKeys)[i];
    if (((invKey._a.empty()) || (invKey._a == cacheKey._a)) &&
        ((invKey._b == 0) || (invKey._b == cacheKey._b)) &&
        ((invKey._c.empty()) || (invKey._c == cacheKey._c)))
    {
      result += DataAccessObject<MinFareApplKey, std::vector<MinFareAppl*> >::invalidate(cacheKey);
    }
  }
  return result;
}

sfc::CompressedData*
MinFareApplDAO::compress(const std::vector<MinFareAppl*>* vect) const
{
  return compressVector(vect);
}

std::vector<MinFareAppl*>*
MinFareApplDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<MinFareAppl>(compressed);
}

std::string
MinFareApplDAO::_name("MinFareAppl");
std::string
MinFareApplDAO::_cacheClass("MinFares");
DAOHelper<MinFareApplDAO>
MinFareApplDAO::_helper(_name);
MinFareApplDAO* MinFareApplDAO::_instance = nullptr;

///////////////////////////////////////////////////////
// Historical DAO Object
///////////////////////////////////////////////////////
log4cxx::LoggerPtr
MinFareApplHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.MinFareApplHistoricalDAO"));

MinFareApplHistoricalDAO&
MinFareApplHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<MinFareAppl*>&
MinFareApplHistoricalDAO::get(DeleteList& del,
                              const VendorCode& textTblVendor,
                              int textTblItemNo,
                              const CarrierCode& governingCarrier,
                              const DateTime& date,
                              const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  MinFareApplHistoricalKey key(textTblVendor, textTblItemNo, governingCarrier);
  DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<MinFareAppl*>* ret = new std::vector<MinFareAppl*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveHist<MinFareAppl>(date, ticketDate));
  return *ret;
}

std::vector<MinFareAppl*>*
MinFareApplHistoricalDAO::create(MinFareApplHistoricalKey key)
{
  std::vector<MinFareAppl*>* ret = new std::vector<MinFareAppl*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get("Historical", true);
  try
  {
    QueryGetMinFareApplBaseHistorical mfab(dbAdapter->getAdapter());
    mfab.findMinFareAppl(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in MinFareApplHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}
void
MinFareApplHistoricalDAO::destroy(MinFareApplHistoricalKey key, std::vector<MinFareAppl*>* recs)
{
  std::vector<MinFareAppl*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
MinFareApplHistoricalDAO::compress(const std::vector<MinFareAppl*>* vect) const
{
  return compressVector(vect);
}

std::vector<MinFareAppl*>*
MinFareApplHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<MinFareAppl>(compressed);
}

std::string
MinFareApplHistoricalDAO::_name("MinFareApplHistorical");
std::string
MinFareApplHistoricalDAO::_cacheClass("MinFares");
DAOHelper<MinFareApplHistoricalDAO>
MinFareApplHistoricalDAO::_helper(_name);
MinFareApplHistoricalDAO* MinFareApplHistoricalDAO::_instance = nullptr;

} // namespace tse
