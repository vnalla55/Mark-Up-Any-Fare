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
#include "DataModel/RepriceCache.h"

#include "Common/Assert.h"
#include "Common/Global.h"
#include "Common/Hasher.h"

namespace tse
{
bool
operator==(const RepriceCache::Key& l, const RepriceCache::Key& r)
{
  if (l.tvlSeg != r.tvlSeg)
    return false;
  if (l.ticketingDate != r.ticketingDate)
    return false;
  if (l.carrierOverride != r.carrierOverride)
    return false;
  if (l.extraPaxType != r.extraPaxType)
    return false;
  if (l.fmDirectionOverride != r.fmDirectionOverride)
    return false;
  if (l.globalDirectionOverride != r.globalDirectionOverride)
    return false;
  if (l.wpncsFlagIndicator != r.wpncsFlagIndicator)
    return false;
  if (l.optionsFareFamilyType != r.optionsFareFamilyType)
    return false;
  if (l.skipRuleValidation != r.skipRuleValidation)
    return false;
  if (l.retrieveFbrFares != r.retrieveFbrFares)
    return false;
  if (l.retrieveNegFares != r.retrieveNegFares)
    return false;
  if (l.privateFareCheck != r.privateFareCheck)
    return false;

  return true;
}

size_t
RepriceCache::Hash::operator()(const Key& key) const
{
  Hasher h(Global::hasherMethod());

  for (const auto& elem : key.tvlSeg)
    h << (uint64_t)elem;
  h << key.ticketingDate.getIntRep();
  if (key.carrierOverride)
    h << *key.carrierOverride;
  h << key.extraPaxType;
  h << (char) key.fmDirectionOverride;
  if (key.globalDirectionOverride)
    h << uint8_t(*key.globalDirectionOverride);
  h << key.wpncsFlagIndicator;
  h << key.optionsFareFamilyType;
  h << key.skipRuleValidation;
  h << key.retrieveFbrFares;
  h << key.retrieveNegFares;
  h << key.privateFareCheck;

  return h.hash();
}

RepriceCache::Result
RepriceCache::get(const Key& key)
{
  std::unique_lock<std::mutex> lock(_mutex);

  Result result;
  _queue.wait(lock, [this, &key, &result]()
  {
    auto p = _map.insert(std::make_pair(key, nullptr));

    if (p.second)
    {
      // Cache miss - the user of get() will fill it later.
      result = Result(*this, &p.first->second);
      return true;
    }

    if (RepricingTrx* trx = p.first->second)
    {
      // Cache hit - the element is complete.
      result = Result(*this, trx);
      return true;
    }

    // Wait for the other thread to fill.
    return false;
  });

  return result;
}

void
RepriceCache::finish(RepricingTrx** place, RepricingTrx* trx)
{
  std::unique_lock<std::mutex> lock(_mutex);

  if (!trx)
  {
    using KV = Map::value_type;
    const auto it = std::find_if(_map.begin(), _map.end(), [place](KV& kv)
    {
      return &kv.second == place;
    });

    TSE_ASSERT(it != _map.end());
    _map.erase(it);
  }
  else
  {
    *place = trx;
  }

  _queue.notify_all();
}
}
