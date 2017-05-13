#include "DataModel/FareMarketCurrencyKey.h"

#include <set>

using namespace std;
using namespace tse;

FareMarketCurrencyKey::FareMarketCurrencyKey(const PaxTypeCode& paxType,
                                             const int& dir,
                                             const CurrencyCode& currency)
  : _paxTypeCode(paxType), _directionality(dir), _currencyCode(currency)
{
}

FareMarketCurrencyKey::FareMarketCurrencyKey(const PaxTypeCode& paxType, const int& dir)
  : _paxTypeCode(paxType), _directionality(dir)
{
}

FareMarketCurrencyKey::FareMarketCurrencyKey(const FareMarketCurrencyKey& key)
  : _paxTypeCode(key._paxTypeCode),
    _directionality(key._directionality),
    _currencyCode(key._currencyCode)

{
}

bool
FareMarketCurrencyKey::
operator<(const FareMarketCurrencyKey& key) const
{
  if (_paxTypeCode < key._paxTypeCode)
    return true;
  if (key._paxTypeCode < _paxTypeCode)
    return false;
  if (_currencyCode < key._currencyCode)
    return true;
  if (key._currencyCode < _currencyCode)
    return false;
  if (_directionality < key._directionality)
    return true;
  if (key._directionality < _directionality)
    return false;

  return false;
}

bool
FareMarketCurrencyKey::CompareKeys::
operator()(const FareMarketCurrencyKey& key1, const FareMarketCurrencyKey& key2)
{
  if (key1._paxTypeCode < key2._paxTypeCode)
    return true;
  if (key2._paxTypeCode < key1._paxTypeCode)
    return false;
  if (key1._currencyCode < key2._currencyCode)
    return true;
  if (key2._currencyCode < key1._currencyCode)
    return false;
  if (key1._directionality < key2._directionality)
    return true;
  if (key2._directionality < key1._directionality)
    return false;

  return false;
}
