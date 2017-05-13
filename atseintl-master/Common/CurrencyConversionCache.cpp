#include "Common/CurrencyConversionCache.h"

#include "Common/DateTime.h"
#include "DBAccess/DataHandle.h"

namespace tse
{

CurrencyConversionCache::CurrencyConversionCache(DataHandle& dh) : _dh(dh) {}

const NUCInfo*
CurrencyConversionCache::getNUCInfo(const CurrencyCode& currency,
                                    const CarrierCode& carrier,
                                    const DateTime& ticketDate)
{
  NUCInfo*& res = _nucCache[currency][carrier][ticketDate];
  if (res == nullptr)
  {
    res = _dh.getNUCFirst(currency, carrier, ticketDate);
  }

  return res;
}
}
