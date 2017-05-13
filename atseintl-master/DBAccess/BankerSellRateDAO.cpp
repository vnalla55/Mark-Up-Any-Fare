//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DBAccess/BankerSellRateDAO.h"

#include "Common/Logger.h"
#include "DBAccess/BankerSellRate.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DBHistoryServer.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetBSR.h"

#include <type_traits>
#include <utility>

namespace tse
{
log4cxx::LoggerPtr
BankerSellRateDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.BankerSellRateDAO"));

BankerSellRateDAO&
BankerSellRateDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

struct BankerSellRateDAO::compareCur
{
  bool operator()(const BankerSellRate* bsr, const CurrencyCode& cur) { return (bsr->cur() < cur); }
  bool operator()(const CurrencyCode& cur, const BankerSellRate* bsr) { return (cur < bsr->cur()); }
};
// DVD
const std::vector<BankerSellRate*>&
getBankerSellRateData(const CurrencyCode& primeCur,
                      const CurrencyCode& cur,
                      const DateTime& date,
                      DeleteList& deleteList,
                      const DateTime& ticketDate,
                      bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    BankerSellRateHistoricalDAO& dao = BankerSellRateHistoricalDAO::instance();
    return dao.get(deleteList, primeCur, cur, date, ticketDate);
  }
  else
  {
    BankerSellRateDAO& dao = BankerSellRateDAO::instance();
    return dao.get(deleteList, primeCur, cur, date, ticketDate);
  }
}

FilterItRange<BankerSellRate, std::pair<CommonContext, CurrencyCode>>
getBankerSellRateDataRange(const CurrencyCode& primeCur,
                           const CurrencyCode& cur,
                           const DateTime& date,
                           DeleteList& deleteList,
                           const DateTime& ticketDate,
                           bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    BankerSellRateHistoricalDAO& dao = BankerSellRateHistoricalDAO::instance();
    return dao.getRange(deleteList, primeCur, cur, date, ticketDate);
  }
  else
  {
    BankerSellRateDAO& dao = BankerSellRateDAO::instance();
    return dao.getRange(deleteList, primeCur, cur, date, ticketDate);
  }
}

FilterItRange<BankerSellRate, std::pair<CommonContext, CurrencyCode>>
BankerSellRateDAO::getRange(DeleteList& del,
                            const CurrencyCode& primeCur,
                            const CurrencyCode& cur,
                            const DateTime& date,
                            const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(primeCur);
  del.copy(ptr);

  return makeFilterIteratorRange(
      ptr.get(),
      std::make_pair(CommonContext(date, ticketDate), cur),
      [](const BankerSellRate* bsr, const std::pair<CommonContext, CurrencyCode>& context)
      { return bsr->cur() == context.second && IsEffectiveG<BankerSellRate>(context.first)(bsr); });
}

const std::vector<BankerSellRate*>&
BankerSellRateDAO::get(DeleteList& del,
                       const CurrencyCode& primeCur,
                       const CurrencyCode& cur,
                       const DateTime& date,
                       const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(primeCur);
  return *(searchAndFilter(
      del, ptr, compareCur(), cur, IsNotEffectiveG<BankerSellRate>(date, ticketDate)));
}

std::vector<BankerSellRate*>*
BankerSellRateDAO::create(CurrencyKey key)
{
  std::vector<BankerSellRate*>* ret = new std::vector<BankerSellRate*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetBSR bsr(dbAdapter->getAdapter());
    bsr.findBSR(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in BankerSellRateDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
BankerSellRateDAO::destroy(CurrencyKey key, std::vector<BankerSellRate*>* recs)
{
  destroyContainer(recs);
}

CurrencyKey
BankerSellRateDAO::createKey(BankerSellRate* info)
{
  return CurrencyKey(info->primeCur());
}

void
BankerSellRateDAO::load()
{
  StartupLoaderNoDB<BankerSellRate, BankerSellRateDAO>();
}

sfc::CompressedData*
BankerSellRateDAO::compress(const std::vector<BankerSellRate*>* vect) const
{
  return compressVector(vect);
}

std::vector<BankerSellRate*>*
BankerSellRateDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<BankerSellRate>(compressed);
}

std::string
BankerSellRateDAO::_name("BankerSellRate");
std::string
BankerSellRateDAO::_cacheClass("Currency");

DAOHelper<BankerSellRateDAO>
BankerSellRateDAO::_helper(_name);

BankerSellRateDAO* BankerSellRateDAO::_instance = nullptr;

// Historical Stuff /////////////////////////////////////////////////////////////////
log4cxx::LoggerPtr
BankerSellRateHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.BankerSellRateHistoricalDAO"));

BankerSellRateHistoricalDAO&
BankerSellRateHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

struct BankerSellRateHistoricalDAO::IsNotCur
{
  const CurrencyCode _cur;
  IsNotEffectiveHist<BankerSellRate> _isNotEffectiveHist;

  IsNotCur(const CurrencyCode& cur, const DateTime& date, const DateTime& ticketDate)
    : _cur(cur), _isNotEffectiveHist(date, ticketDate)
  {
  }

  bool operator()(const BankerSellRate* bsr)
  {
    return bsr->cur() != _cur || _isNotEffectiveHist(bsr);
  }
};

const std::vector<BankerSellRate*>&
BankerSellRateHistoricalDAO::get(DeleteList& del,
                                 const CurrencyCode& primeCur,
                                 const CurrencyCode& cur,
                                 const DateTime& ticketDate,
                                 const DateTime& date)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  BankerSellRateHistoricalKey key(primeCur);
  DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<BankerSellRate*>* ret = new std::vector<BankerSellRate*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), IsNotCur(cur, date, ticketDate));
  return *ret;
}

FilterItRange<BankerSellRate, std::pair<CommonContext, CurrencyCode>>
BankerSellRateHistoricalDAO::getRange(DeleteList& del,
                                      const CurrencyCode& primeCur,
                                      const CurrencyCode& cur,
                                      const DateTime& ticketDate,
                                      const DateTime& date)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  BankerSellRateHistoricalKey key(primeCur);
  DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  return makeFilterIteratorRange(ptr.get(),
                                 std::make_pair(CommonContext(date, ticketDate), cur),
                                 [](const BankerSellRate* bsr,
                                    const std::pair<CommonContext, CurrencyCode>& context)
                                 {
    return bsr->cur() == context.second && !IsNotEffectiveHist<BankerSellRate>(context.first)(bsr);
  });
}

std::vector<BankerSellRate*>*
BankerSellRateHistoricalDAO::create(BankerSellRateHistoricalKey key)
{
  std::vector<BankerSellRate*>* ret = new std::vector<BankerSellRate*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get("Historical", true);
  try
  {
    QueryGetBSRHistorical bsr(dbAdapter->getAdapter());
    bsr.findBSR(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in BankerSellRateHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
BankerSellRateHistoricalDAO::destroy(BankerSellRateHistoricalKey key,
                                     std::vector<BankerSellRate*>* recs)
{
  std::vector<BankerSellRate*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
BankerSellRateHistoricalDAO::compress(const std::vector<BankerSellRate*>* vect) const
{
  return compressVector(vect);
}

std::vector<BankerSellRate*>*
BankerSellRateHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<BankerSellRate>(compressed);
}

std::string
BankerSellRateHistoricalDAO::_name("BankerSellRateHistorical");
std::string
BankerSellRateHistoricalDAO::_cacheClass("Currency");
DAOHelper<BankerSellRateHistoricalDAO>
BankerSellRateHistoricalDAO::_helper(_name);
BankerSellRateHistoricalDAO* BankerSellRateHistoricalDAO::_instance = nullptr;

} // namespace tse
