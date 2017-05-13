//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#include "DBAccess/MultiTransportCityDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/MultiTransport.h"
#include "DBAccess/Queries/QueryGetMultiTransport.h"

#include <pthread.h>

namespace tse
{
log4cxx::LoggerPtr
MultiTransportCityDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.MultiTransportCityDAO"));

MultiTransportCityDAO&
MultiTransportCityDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const LocCode
getMultiTransportCityData(const LocCode& locCode,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical)
{
  if (isHistorical)
  {
    MultiTransportCityHistoricalDAO& dao = MultiTransportCityHistoricalDAO::instance();
    return dao.get(deleteList, locCode, ticketDate);
  }
  else
  {
    MultiTransportCityDAO& dao = MultiTransportCityDAO::instance();
    return dao.get(deleteList, locCode, ticketDate);
  }
}

const LocCode
MultiTransportCityDAO::get(DeleteList& del, const LocCode& locCode, const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(LocCodeKey(locCode));
  std::vector<MultiTransport*>::iterator i = ptr->begin();
  for (; i != ptr->end(); ++i)
    if ((*i)->intlAppl() == 'Y')
      return (*i)->multitranscity();
  return "";
}

struct MultiTransportCityDAO::matchKeys : public std::unary_function<MultiTransport*, bool>
{
public:
  const CarrierCode _carrier;
  const GeoTravelType _tvlType;
  const DateTime& _tvlDate;

  matchKeys(const CarrierCode carrier, GeoTravelType tvlType, const DateTime& tvlDate)
    : _carrier(carrier), _tvlType(tvlType), _tvlDate(tvlDate)
  {
  }

  bool operator()(MultiTransport* rec) const
  {
    return (rec->carrier() == _carrier &&
            (((_tvlType == GeoTravelType::Domestic || _tvlType == GeoTravelType::Transborder) && rec->domAppl() == 'Y') ||
             ((_tvlType != GeoTravelType::Domestic && _tvlType != GeoTravelType::Transborder) && rec->intlAppl() == 'Y')) &&
            (_tvlDate > rec->effDate() && _tvlDate < rec->expireDate()));
  }
}; // lint !e1510

const std::vector<MultiTransport*>&
getMultiTransportCityData(const LocCode& locCode,
                          const CarrierCode& carrierCode,
                          GeoTravelType tvlType,
                          const DateTime& tvlDate,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    MultiTransportCityHistoricalDAO& dao = MultiTransportCityHistoricalDAO::instance();
    return dao.get(deleteList, locCode, carrierCode, tvlType, tvlDate, ticketDate);
  }
  else
  {
    MultiTransportCityDAO& dao = MultiTransportCityDAO::instance();
    return dao.get(deleteList, locCode, carrierCode, tvlType, tvlDate, ticketDate);
  }
}

const std::vector<MultiTransport*>&
MultiTransportCityDAO::get(DeleteList& del,
                           const LocCode& locCode,
                           const CarrierCode& carrier,
                           GeoTravelType tvlType,
                           const DateTime& tvlDate,
                           const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  std::vector<MultiTransport*>* ret = new std::vector<MultiTransport*>;
  del.adopt(ret);
  DAOCache::pointer_type ptr = cache().get(LocCodeKey(locCode));
  del.copy(ptr);
  remove_copy_if(
      ptr->begin(), ptr->end(), back_inserter(*ret), not1(matchKeys(carrier, tvlType, tvlDate)));
  if (ret->empty())
  {
    remove_copy_if(
        ptr->begin(), ptr->end(), back_inserter(*ret), not1(matchKeys("", tvlType, tvlDate)));
  }
  return *ret;
}

// this is our most often used DAO, so we have some special performance
// optimizations here.
//
// The most common use of the DAO is to find the city a certain location
// is in. So we specialize that in getLocCode(). This function can
// be called with a certain carrier to see if that carrier overrides
// the default, but most of the time it is called with an empty carrier.
//
// We thus cache results for empty carriers in a simple cache that
// uses a perfect hash map to store the results. This hash map is
// duplicated 'NumLocCodeCache' times to reduce contention between
// different threads.
//
// Anytime there is an ER update, we will clear this entire cache.
// Updates to the multi trans city table shouldn't happen frequently
// enough for this to be much of an issue.
namespace
{
typedef std::array<char, 3> CachedLoc;

#if __BYTE_ORDER == __BIG_ENDIAN
struct ItemStruct // 4B
{
  CachedLoc _loc; // 3B
  uint8_t _reset;
};
#else
struct ItemStruct // 4B
{
  uint8_t _reset;
  CachedLoc _loc; // 3B
};
#endif

typedef uint32_t Atomic;

union Item
{
  Atomic _atomic{0};
  ItemStruct _content;
};

struct LocCodeCache
{
  void reset()
  {
    Item item, newItem;
    for (auto& i: _cache)
    {
      // Replace _atomic with _reset + 1.
      // Effectively this zeroes all bytes in _atomic except the one occupied by _reset.
      // The latter is incremented by 1 (modulo 256) to distinguish between last 255 resets.
      item = i.load(std::memory_order_relaxed);
      newItem._atomic = static_cast<uint8_t>(item._content._reset + 1);
      // Only reset the item if it did not change in the meantime, retry otherwise.
      while (!i.compare_exchange_strong(item, newItem, std::memory_order_relaxed))
      {
        newItem._atomic = static_cast<uint8_t>(item._content._reset + 1);
      }
    }
  }

  typedef std::atomic<Item> AtomicItem;
  std::array<AtomicItem, 26 * 26 * 26> _cache;
};

LocCodeCache locCache;

// a value to indicate that we have found that this entry cannot
// be cached because it is too complicated. (Perhaps it differs
// for domestic vs international, or changes over time). If this
// entry is found, the code will do a full calculation every time.
const CachedLoc InvalidCarrier{'Z'};
}

const LocCode
getMultiTransportCityCodeData(const LocCode& locCode,
                              const CarrierCode& carrierCode,
                              GeoTravelType tvlType,
                              const DateTime& tvlDate,
                              const DateTime& ticketDate,
                              bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    MultiTransportCityHistoricalDAO& dao = MultiTransportCityHistoricalDAO::instance();
    return dao.getLocCode(locCode, carrierCode, tvlType, tvlDate, ticketDate);
  }
  else
  {
    MultiTransportCityDAO& dao = MultiTransportCityDAO::instance();
    return dao.getLocCode(locCode, carrierCode, tvlType, tvlDate);
  }
}

const LocCode
MultiTransportCityDAO::getLocCode(const LocCode& locCode,
                                  const CarrierCode& carrier,
                                  GeoTravelType tvlType,
                                  const DateTime& tvlDate)
{
  unsigned int hash = 0;
  bool tryCache = false;
  LocCodeCache::AtomicItem* item = nullptr;
  Item itemCopy, newItem;

  if (carrier.empty())
  {
    hash = 26 * 26 * (locCode[0] - 'A') + 26 * (locCode[1] - 'A') + (locCode[2] - 'A');
    item = &locCache._cache[hash];
    itemCopy = item->load(std::memory_order_relaxed);

    if (itemCopy._content._loc != InvalidCarrier)
    {
      if (itemCopy._content._loc[0] != '\0')
      {
        return LocCode(itemCopy._content._loc.data(), itemCopy._content._loc.size());
      }
      else if (!Global::allowHistorical())
      {
        tryCache = true;
      }
    }
  }

  DAOCache::pointer_type ptr = cache().get(locCode);

  // we're going to try and see if we can generate the result and
  // put it into the local cache to speed future queries.
  //
  // Invariant:
  // (tryCache == true) => (item and itemCopy have been assigned)
  if (tryCache)
  {
    LocCode res;
    bool oneCode = true;
    bool gotRecord = false;
    bool gotIntl = false;
    bool gotDom = false;

    // we put something into the local cache if either it's
    // not present at all (in which case we just return the
    // passed-in value), or if it is present, and uses the
    // same code for both international and domestic,
    // it never expires, and its effective date is in the past
    DateTime effDate = DateTime::emptyDate();
    for (std::vector<MultiTransport*>::const_iterator i = ptr->begin(); i != ptr->end(); ++i)
    {
      if ((*i)->carrier().empty())
      {
        if (res.empty())
        {
          res = (*i)->multitranscity();
        }
        else if (res != (*i)->multitranscity())
        {
          oneCode = false;
          break;
        }

        if ((*i)->expireDate().isPosInfinity() == false)
        {
          oneCode = false;
          break;
        }

        if (effDate.isEmptyDate() || (*i)->effDate() < effDate)
        {
          effDate = (*i)->effDate();
        }

        gotRecord = true;
        if ((*i)->intlAppl() == 'Y')
        {
          gotIntl = true;
        }

        if ((*i)->domAppl() == 'Y')
        {
          gotDom = true;
        }
      }
    }

    if (oneCode && (gotRecord == false || (gotIntl && gotDom)) &&
        (effDate.isEmptyDate() || (effDate < tvlDate && effDate < DateTime::localTime())))
    {
      if (res.empty())
      {
        res = locCode;
      }

      for (int j: {0, 1, 2})
        newItem._content._loc[j] = res[j];
      // only update cache if the item was not changed by another thread
      item->compare_exchange_strong(itemCopy, newItem, std::memory_order_relaxed);

      return res;
    }
    else
    {
      newItem._content._loc = InvalidCarrier;
      // only update cache if the item was not changed by another thread
      item->compare_exchange_strong(itemCopy, newItem, std::memory_order_relaxed);
    }
  }

  std::vector<MultiTransport*>::const_iterator itor =
      std::find_if(ptr->begin(), ptr->end(), matchKeys(carrier, tvlType, tvlDate));
  if (itor == ptr->end() && !carrier.empty())
  {
    itor = std::find_if(ptr->begin(), ptr->end(), matchKeys("", tvlType, tvlDate));
  }

  if (itor != ptr->end())
  {
    return (*itor)->multitranscity();
  }
  else
  {
    return locCode;
  }
}

size_t
MultiTransportCityDAO::invalidate(const ObjectKey& objectKey)
{
  size_t result(DataAccessObject<LocCodeKey, std::vector<MultiTransport*>, false>::invalidate(objectKey));

  locCache.reset();

  return result;
}

LocCodeKey
MultiTransportCityDAO::createKey(MultiTransport* info)
{
  return LocCodeKey(info->multitransLoc());
}

std::vector<MultiTransport*>*
MultiTransportCityDAO::create(LocCodeKey key)
{
  std::vector<MultiTransport*>* ret = new std::vector<MultiTransport*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetMultiTransportCity mtc(dbAdapter->getAdapter());
    mtc.findMultiTransportCity(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in MultiTransportDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
MultiTransportCityDAO::destroy(LocCodeKey key, std::vector<MultiTransport*>* recs)
{
  destroyContainer(recs);
}

void
MultiTransportCityDAO::load()
{
  StartupLoader<QueryGetAllMultiTransportCity, MultiTransport, MultiTransportCityDAO>();
}

sfc::CompressedData*
MultiTransportCityDAO::compress(const std::vector<MultiTransport*>* vect) const
{
  return compressVector(vect);
}

std::vector<MultiTransport*>*
MultiTransportCityDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<MultiTransport>(compressed);
}

std::string
MultiTransportCityDAO::_name("MultiTransportCity");
std::string
MultiTransportCityDAO::_cacheClass("Common");

DAOHelper<MultiTransportCityDAO>
MultiTransportCityDAO::_helper(_name);

MultiTransportCityDAO* MultiTransportCityDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: MultiTransportCityHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
MultiTransportCityHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.MultiTransportCityHistoricalDAO"));
MultiTransportCityHistoricalDAO&
MultiTransportCityHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

struct MultiTransportCityHistoricalDAO::matchKeysH
    : public std::unary_function<MultiTransport*, bool>
{
public:
  const CarrierCode _carrier;
  const GeoTravelType _tvlType;
  const DateTime& _tvlDate;
  const DateTime& _tktDate;

  IsEffectiveH<MultiTransport> _isEffectiveH;

  matchKeysH(const CarrierCode carrier,
             GeoTravelType tvlType,
             const DateTime& tvlDate,
             const DateTime& ticketDate)
    : _carrier(carrier),
      _tvlType(tvlType),
      _tvlDate(tvlDate),
      _tktDate(ticketDate),
      _isEffectiveH(_tvlDate, _tktDate)
  {
  }

  bool operator()(MultiTransport* rec) const
  {
    return (rec->carrier() == _carrier &&
            (((_tvlType == GeoTravelType::Domestic || _tvlType == GeoTravelType::Transborder) && rec->domAppl() == 'Y') ||
             ((_tvlType != GeoTravelType::Domestic && _tvlType != GeoTravelType::Transborder) && rec->intlAppl() == 'Y')) &&
            (_isEffectiveH(rec)));
  }
}; // lint !e1510

const std::vector<MultiTransport*>&
MultiTransportCityHistoricalDAO::get(DeleteList& del,
                                     const LocCode& locCode,
                                     const CarrierCode& carrier,
                                     GeoTravelType tvlType,
                                     const DateTime& tvlDate,
                                     const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  MultiTransportCityHistoricalKey cacheKey(locCode);
  DAOUtils::getDateRange(ticketDate, cacheKey._b, cacheKey._c, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(cacheKey);
  del.copy(ptr);
  std::vector<MultiTransport*>* ret = new std::vector<MultiTransport*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 not1(matchKeysH(carrier, tvlType, tvlDate, ticketDate)));
  if (ret->empty())
  {
    remove_copy_if(ptr->begin(),
                   ptr->end(),
                   back_inserter(*ret),
                   not1(matchKeysH("", tvlType, tvlDate, ticketDate)));
  }
  return *ret;
}

const LocCode
MultiTransportCityHistoricalDAO::get(DeleteList& del,
                                     const LocCode& locCode,
                                     const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  std::vector<MultiTransport*>* ret = new std::vector<MultiTransport*>;
  del.adopt(ret);
  MultiTransportCityHistoricalKey cacheKey(locCode);
  DAOUtils::getDateRange(ticketDate, cacheKey._b, cacheKey._c, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(cacheKey);
  del.copy(ptr);
  remove_copy_if(
      ptr->begin(), ptr->end(), back_inserter(*ret), IsNotCurrentH<MultiTransport>(ticketDate));
  std::vector<MultiTransport*>::iterator i = ret->begin();
  for (; i != ret->end(); ++i)
    if ((*i)->intlAppl() == 'Y')
      return (*i)->multitranscity();
  return "";
}

const LocCode
MultiTransportCityHistoricalDAO::getLocCode(const LocCode& locCode,
                                            const CarrierCode& carrier,
                                            GeoTravelType tvlType,
                                            const DateTime& tvlDate,
                                            const DateTime& tktDate)
{
  // since this is historical version of this DAO
  // it will use sandard method of accessing cache
  MultiTransportCityHistoricalKey cacheKey(locCode);
  DAOUtils::getDateRange(tktDate, cacheKey._b, cacheKey._c, _cacheBy);

  LocCode res;
  DAOCache::pointer_type ptr = cache().get(cacheKey);

  std::vector<MultiTransport*>::const_iterator itor =
      std::find_if(ptr->begin(), ptr->end(), matchKeysH(carrier, tvlType, tvlDate, tktDate));

  if (itor != ptr->end())
  {
    return (*itor)->multitranscity();
  }
  else if (!carrier.empty())
  {
    itor = std::find_if(ptr->begin(), ptr->end(), matchKeysH("", tvlType, tvlDate, tktDate));
    if (itor != ptr->end())
    {
      return (*itor)->multitranscity();
    }
  }

  return locCode;
}

std::vector<MultiTransport*>*
MultiTransportCityHistoricalDAO::create(MultiTransportCityHistoricalKey key)
{
  std::vector<MultiTransport*>* ret = new std::vector<MultiTransport*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetMultiTransportCityHistorical mtc(dbAdapter->getAdapter());
    mtc.findMultiTransportCity(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in MultiTransportDAOHistorical::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
MultiTransportCityHistoricalDAO::destroy(MultiTransportCityHistoricalKey key,
                                         std::vector<MultiTransport*>* recs)
{
  destroyContainer(recs);
}

sfc::CompressedData*
MultiTransportCityHistoricalDAO::compress(const std::vector<MultiTransport*>* vect) const
{
  return compressVector(vect);
}

std::vector<MultiTransport*>*
MultiTransportCityHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<MultiTransport>(compressed);
}

std::string
MultiTransportCityHistoricalDAO::_name("MultiTransportCityHistorical");
std::string
MultiTransportCityHistoricalDAO::_cacheClass("Common");
DAOHelper<MultiTransportCityHistoricalDAO>
MultiTransportCityHistoricalDAO::_helper(_name);

MultiTransportCityHistoricalDAO* MultiTransportCityHistoricalDAO::_instance = nullptr;

} // namespace tse
