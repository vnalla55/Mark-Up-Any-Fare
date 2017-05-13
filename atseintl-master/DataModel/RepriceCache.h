//-------------------------------------------------------------------
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------
#pragma once

#include "Common/DateTime.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

#include <condition_variable>
#include <mutex>
#include <unordered_map>

#include <assert.h>

#include <boost/optional.hpp>

namespace tse
{
class RepricingTrx;

class RepriceCache
{
public:
  struct Key
  {
    Key() {}

    std::vector<TravelSeg*> tvlSeg;
    DateTime ticketingDate;
    boost::optional<CarrierCode> carrierOverride = boost::none;
    PaxTypeCode extraPaxType;
    FMDirection fmDirectionOverride;
    boost::optional<GlobalDirection> globalDirectionOverride = boost::none;
    Indicator wpncsFlagIndicator;
    char optionsFareFamilyType;
    bool skipRuleValidation;
    bool retrieveFbrFares;
    bool retrieveNegFares;
    bool privateFareCheck;

    friend bool operator==(const Key& l, const Key& r);
  };

  struct Hash
  {
    size_t operator()(const Key& key) const;
  };

  using Map = std::unordered_map<Key, RepricingTrx*, Hash>;

  class Result
  {
    friend class RepriceCache;

    Result(RepriceCache& cache, RepricingTrx* result) : _cache(&cache), _result(result) {}
    Result(RepriceCache& cache, RepricingTrx** place) : _cache(&cache), _place(place) {}

  public:
    Result() {}
    ~Result() { clear(); }

    Result(const Result&) = delete;
    Result& operator=(const Result&) = delete;
    Result(Result&& o) : _cache(o._cache), _result(o._result), _place(o._place)
    {
      o._place = nullptr;
    }
    Result& operator=(Result&& o)
    {
      clear();
      _cache = o._cache;
      _result = o._result;
      _place = o._place;
      o._place = nullptr;
      return *this;
    }

    void clear()
    {
      if (!hit())
      {
        _cache->finish(_place, _result);
        _place = nullptr;
      }
    }

    bool hit() const { return !_place; }

    RepricingTrx* result()
    {
      assert(hit());
      return _result;
    }
    void fill(RepricingTrx* trx)
    {
      assert(!hit());
      _result = trx;
    }

  private:
    RepriceCache* _cache = nullptr;
    RepricingTrx* _result = nullptr;
    RepricingTrx** _place = nullptr;
  };

  // The user of get() has the responsibility to fill() the cache when the result is not a hit().
  // If it cannot, it should leave the result as it is. The ~Result() will free the place for
  // other users to fill.
  Result get(const Key& key);

private:
  std::mutex _mutex;
  Map _map;
  std::condition_variable _queue;

  void finish(RepricingTrx** place, RepricingTrx* trx);
};
}
