#pragma once

#include "Allocator/TrxMalloc.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/LocUtilImpl.h"
#include "Common/Money.h"
#include "Common/PaxTypeUtil.h"
#include "Common/Thread/TseCallableTrxTask.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TSSCacheConst.h"
#include "DBAccess/Loc.h"
#include "Util/Algorithm/Tuple.h"

#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>

#include <cstdint>

namespace tse
{

namespace tsscache
{

extern __thread class TSSStorage* storage;

enum
{
  ATTRIBUTES_SIZE,
  ATTRIBUTES_STATS
};

struct Attributes
{
  Attributes()
    : _size(-1)
    , _stats(false)
  {
  }
  int _size;
  int _stats;
};

Attributes getCacheAttributes(const std::string& name);

template <typename T>
void hash_combine(size_t& hash, const T& value)
{
  boost::hash_combine(hash, value);
}

template <size_t n>
void hash_combine(size_t& hash,
                  const Code<n>& str)
{
  str.hash_combine(hash);
}

template <typename T>
void hash_combine(size_t& hash,
                  const std::vector<T>& vect)
{
  for (const T& elem : vect)
  {
    hash_combine(hash, elem);
  }
}

void printStats(const std::string& name,
                int size,
                long total,
                long noTrxId,
                long reuse,
                long hits,
                int stats);

template <typename T>
void doStats(int size,
             bool valid,
             bool hit,
             int stats,
             int trxId)
{
  static boost::mutex mutex;
  static long total(0);
  static long noTrxId(0);
  static long reuse(0);
  static long hits(0);
  static const std::string name(cacheNameByIndex(T::_cacheIndex));
  boost::mutex::scoped_lock lock(mutex);
  ++total;
  if (-1 == trxId)
  {
    ++noTrxId;
  }
  if (valid)
  {
    ++reuse;
    if (hit)
    {
      ++hits;
    }
  }
  printStats(name, size, total, noTrxId, reuse, hits, stats);
}

std::vector<char>* getMemoryBuffer();

struct CacheBase
{
  virtual ~CacheBase()
  {
  }
};

struct TSSStorage
{
  TSSStorage(pthread_key_t key);

  ~TSSStorage();
  
  CacheBase* _caches[NUMBER_OF_CACHES];
  std::vector<char>* _memoryBuffer;
  pthread_key_t _key;
};

TSSStorage* initStorage();

size_t initializeMemoryBufferSize();

inline size_t memoryBufferSize()
{
  static const size_t size(initializeMemoryBufferSize());
  return size;
}

inline std::vector<char>* getMemoryBuffer()
{
  if (nullptr == storage && nullptr == initStorage())
  {
    return nullptr;
  }
  std::vector<char>* memoryBuffer(storage->_memoryBuffer);
  if (nullptr == memoryBuffer)
  {
    const MallocContextDisabler context;
    memoryBuffer = new std::vector<char>(memoryBufferSize());
    storage->_memoryBuffer = memoryBuffer;
  }
  return memoryBuffer;
}

template <typename T>
struct Cache : public CacheBase
{
  explicit Cache(size_t size)
    : _array(size)
  {
  }

  virtual ~Cache()
  {
  }

  template <class... KeyMember>
  static T* processEntry(bool& hit,
                         int trxId,
                         const KeyMember&... val)
  {
    static Attributes attrs(getCacheAttributes(cacheNameByIndex(T::_cacheIndex)));
    int size(attrs._size);
    int stats(attrs._stats);
    T* result(nullptr);
    bool valid(false);
    Cache* cache(nullptr);
    if (trxId > -1 && (cache = instance(size)) != nullptr)
    {
      size_t hash(0);
      alg::visit_tuple(std::tie(val...), [&](const auto& member) { hash_combine(hash, member); });
      T& entry(cache->_array[hash % size]);
      valid = trxId == entry._trxId;
      hit = valid && entry.equal(val...);
      result = &entry;
    }
    if (UNLIKELY(stats))
    {
      doStats<T>(size, valid, hit, stats, trxId);
    }
    return result;
  }

  static Cache* instance(int size)
  {
    if (storage)
    {
      CacheBase* instance(storage->_caches[T::_cacheIndex]);
      if (instance)
      {
        return static_cast<Cache*>(instance);
      }
    }
    if (size <= 0 || (nullptr == storage && nullptr == initStorage()))
    {
      return nullptr;
    }
    CacheBase*& instance(storage->_caches[T::_cacheIndex]);
    if (nullptr == instance)
    {
      const MallocContextDisabler context;
      instance = new Cache(size);
    }
    return static_cast<Cache*>(instance);
  }

  std::vector<T> _array;
};

struct Entry
{
  Entry() : _cat(0), _ticketDateInt(0), _isHistorical(false), _trxId(-1) {}
  uint16_t _cat;
  uint64_t _ticketDateInt;
  bool _isHistorical;
  VendorCode _vendor;
  int _trxId;
};

// IsInLoc cache
struct IsInLocEntry : public Entry
{
  IsInLocEntry()
    : _locType(UNKNOWN_LOC),
      _zoneType(0),
      _geoTvlType(GeoTravelType::UnknownGeoTravelType),
      _applType(LocUtil::OTHER),
      _result(false)
  {
  }

  bool equal(const LocCode& city,
             LocTypeCode locType,
             const LocCode& locCode,
             const VendorCode& vendor,
             ZoneType zoneType,
             GeoTravelType geoTvlType,
             LocUtil::ApplicationType applType,
             uint64_t ticketDateInt) const
  {
    return _locType == locType && _zoneType == zoneType && _geoTvlType == geoTvlType &&
           _applType == applType && _ticketDateInt == ticketDateInt && _city == city &&
           _locCode == locCode && _vendor == vendor;
  }

  LocCode _city;
  LocTypeCode _locType;
  LocCode _locCode;
  ZoneType _zoneType;
  GeoTravelType _geoTvlType;
  LocUtil::ApplicationType _applType;
  bool _result;
  static TSSCacheEnum _cacheIndex;
};

inline bool
isInLoc(const LocCode& city,
        LocTypeCode locType,
        const LocCode& locCode,
        const VendorCode& vendor,
        ZoneType zoneType,
        GeoTravelType geoTvlType,
        LocUtil::ApplicationType applType,
        const DateTime& ticketDate)
{
  typedef IsInLocEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t ticketDateInt(ticketDate.getIntRep());
  int trxId(TseCallableTrxTask::getCurrentTrxId());
  EntryType* entry(CacheType::processEntry(
      hit, trxId, city, locType, locCode, vendor, zoneType, geoTvlType, applType, ticketDateInt));
  if (hit)
  {
    return entry->_result;
  }
  bool result(LocUtil::isInLocImpl(
      city, locType, locCode, vendor, zoneType, geoTvlType, applType, ticketDate));
  if (LIKELY(entry))
  {
    entry->_city = city;
    entry->_locType = locType;
    entry->_locCode = locCode;
    entry->_vendor = vendor;
    entry->_zoneType = zoneType;
    entry->_geoTvlType = geoTvlType;
    entry->_applType = applType;
    entry->_ticketDateInt = ticketDateInt;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return result;
}

// IsAnActualPaxInTrx
struct IsAnActualPaxInTrxEntry : public Entry
{
  IsAnActualPaxInTrxEntry() : _result(nullptr) {}

  bool equal(const CarrierCode& carrier, const PaxTypeCode& paxTypeCode) const
  {
    return _carrier == carrier && _paxTypeCode == paxTypeCode;
  }

  CarrierCode _carrier;
  PaxTypeCode _paxTypeCode;
  const PaxType* _result;
  static TSSCacheEnum _cacheIndex;
};

inline const PaxType*
isAnActualPaxInTrx(const PricingTrx& trx,
                   const CarrierCode& carrier,
                   const PaxTypeCode& paxTypeCode,
                   int trxId)

{
  typedef IsAnActualPaxInTrxEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  EntryType* entry(CacheType::processEntry(hit, trxId, carrier, paxTypeCode));
  if (UNLIKELY(hit))
  {
    return entry->_result;
  }
  const PaxType* result(PaxTypeUtil::isAnActualPaxInTrxImpl(trx, carrier, paxTypeCode));
  if (UNLIKELY(entry))
  {
    entry->_carrier = carrier;
    entry->_paxTypeCode = paxTypeCode;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return result;
}

// date
struct DateEntry : public Entry
{
  bool equal(int64_t timeAsNumber) const { return _timeAsNumber == timeAsNumber; }

  int64_t _timeAsNumber;
  boost::gregorian::date _result;
  static TSSCacheEnum _cacheIndex;
};

inline boost::gregorian::date
date(const DateTime& dateTime)
{
  typedef DateEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  int trxId(0);
  int64_t timeAsNumber(dateTime.getIntRep());
  EntryType* entry(CacheType::processEntry(hit, trxId, timeAsNumber));
  if (hit)
  {
    return entry->_result;
  }
  boost::gregorian::date result(dateTime.dateImpl());
  if (LIKELY(entry))
  {
    entry->_timeAsNumber = timeAsNumber;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return result;
}

// CurrencyConversionRound cache

struct CurrencyConversionRoundEntry : public Entry
{
  CurrencyConversionRoundEntry()
    : _valueInput(0)
    , _valueOutput(0)
    , _useInternationalRounding(false)
    , _result(false)
  {
  }

  bool equal(const CurrencyCode& code,
             MoneyAmount valueInput,
             bool useInternationalRounding) const
  {
    return _valueInput == valueInput
           && _useInternationalRounding == useInternationalRounding
           && _code == code;
  }

  CurrencyCode _code;
  MoneyAmount _valueInput;
  MoneyAmount _valueOutput;
  bool _useInternationalRounding;
  bool _result;
  static TSSCacheEnum _cacheIndex;
};

inline bool ccfRound(CurrencyConversionFacade& ccf,
                     Money& target,
                     PricingTrx& trx,
                     bool useInternationalRounding,
                     int trxId)
{
  typedef CurrencyConversionRoundEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  MoneyAmount valueInput(target.value());
  EntryType* entry(CacheType::processEntry(hit,
                                           trxId,
                                           target.code(),
                                           valueInput,
                                           useInternationalRounding));
  if (hit)
  {
    target.value() = entry->_valueOutput;
    return entry->_result;
  }
  bool result(ccf.roundCB(target, trx, useInternationalRounding));
  if (LIKELY(entry))
  {
    entry->_code = target.code();
    entry->_valueInput = valueInput;
    entry->_valueOutput = target.value();
    entry->_useInternationalRounding = useInternationalRounding;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return result;
}

// CurrencyConverterRoundByRule cache

struct CurrencyConverterRoundByRuleEntry : public Entry
{
  CurrencyConverterRoundByRuleEntry()
    : _valueInput(0)
    , _valueOutput(0)
    , _roundingFactor(0)
    , _roundingRule(RoundingRule::NONE)
    , _result(false)
  {
  }

  bool equal(MoneyAmount valueInput,
             RoundingFactor roundingFactor,
             RoundingRule roundingRule) const
  {
    return _valueInput == valueInput
           && _roundingFactor == roundingFactor
           && _roundingRule == roundingRule;
  }

  MoneyAmount _valueInput;
  MoneyAmount _valueOutput;
  RoundingFactor _roundingFactor;
  RoundingRule _roundingRule;
  bool _result;
  static TSSCacheEnum _cacheIndex;
};

inline bool ccRoundByRule(CurrencyConverter& cc,
                          Money& target,
                          RoundingFactor roundingFactor,
                          RoundingRule roundingRule)
{
  typedef CurrencyConverterRoundByRuleEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  int trxId(TseCallableTrxTask::getCurrentTrxId());
  MoneyAmount valueInput(target.value());
  EntryType* entry(CacheType::processEntry(hit,
                                           trxId,
                                           valueInput,
                                           roundingFactor,
                                           roundingRule));
  if (hit)
  {
    target.value() = entry->_valueOutput;
    return entry->_result;
  }
  bool result(cc.roundByRuleCB(target, roundingFactor, roundingRule));
  if (LIKELY(entry))
  {
    entry->_valueInput = valueInput;
    entry->_valueOutput = target.value();
    entry->_roundingFactor = roundingFactor;
    entry->_roundingRule = roundingRule;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return result;
}

// isInZone cache

struct IsInZoneEntry : public Entry
{
  IsInZoneEntry()
    : _zoneType(' ')
    , _applType(LocUtil::OTHER)
    , _geoTvlType(GeoTravelType::UnknownGeoTravelType)
    , _result(false)
  {
  }

  bool equal(const LocCode& locLocCode,
             const VendorCode& vendor,
             const Zone& zone,
             ZoneType zoneType,
             LocUtil::ApplicationType applType,
             GeoTravelType geoTvlType,
             const CarrierCode& carrier,
             uint64_t ticketDateInt) const
  {
    return _ticketDateInt == ticketDateInt
           && _zoneType == zoneType
           && _applType == applType
           && _geoTvlType == geoTvlType
           && _locLocCode == locLocCode
           && _zone == zone
           && _carrier == carrier
           && _vendor == vendor;
  }
  
  LocCode _locLocCode;
  Zone _zone;
  ZoneType _zoneType;
  LocUtil::ApplicationType _applType;
  GeoTravelType _geoTvlType;
  CarrierCode _carrier;
  bool _result;
  static TSSCacheEnum _cacheIndex;
};

inline bool isInZone(const Loc& loc,
                     const VendorCode& vendor,
                     const Zone& zone,
                     ZoneType zoneType,
                     LocUtil::ApplicationType applType,
                     GeoTravelType geoTvlType,
                     const CarrierCode& carrier,
                     DateTime ticketDate)
{
  typedef IsInZoneEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  int trxId(TseCallableTrxTask::getCurrentTrxId());
  const LocCode& locLocCode(loc.loc());
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(hit,
                                           trxId,
                                           locLocCode,
                                           vendor,
                                           zone,
                                           zoneType,
                                           applType,
                                           geoTvlType,
                                           carrier,
                                           ticketDateInt));
  if (hit)
  {
    return entry->_result;
  }
  bool result(LocUtil::isInZoneCB(loc,
                                  vendor,
                                  zone,
                                  zoneType,
                                  applType,
                                  geoTvlType,
                                  carrier,
                                  ticketDate));
  if (LIKELY(entry))
  {
    entry->_locLocCode = locLocCode;
    entry->_vendor = vendor;
    entry->_zone = zone;
    entry->_zoneType = zoneType;
    entry->_applType = applType;
    entry->_geoTvlType = geoTvlType;
    entry->_carrier = carrier;
    entry->_ticketDateInt = ticketDateInt;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return result;
}

} // tsscache

} // tse

