#include "DataModel/CTRWCurrencyKey.h"

#include <set>

using namespace std;
using namespace tse;

CTRWCurrencyKey::CTRWCurrencyKey(const PaxTypeCode& paxType, const CurrencyCode& currency)
  : _paxTypeCode(paxType), _currencyCode(currency)
{
}

/*CTRWCurrencyKey::CTRWCurrencyKey(const CTRWCurrencyKey& key) : _paxTypeCode(key._paxTypeCode),
                                                               _currencyCode(key._currencyCode)

{

}*/

bool
CTRWCurrencyKey::CompareKeys::
operator()(const CTRWCurrencyKey& key1, const CTRWCurrencyKey& key2)
{
  if (key1._paxTypeCode < key2._paxTypeCode)
    return true;
  if (key2._paxTypeCode < key1._paxTypeCode)
    return false;
  if (key1._currencyCode < key2._currencyCode)
    return true;
  if (key2._currencyCode < key1._currencyCode)
    return false;

  return false;
}

bool
CTRWCurrencyKey::
operator<(const CTRWCurrencyKey& key) const
{
  if (_paxTypeCode < key._paxTypeCode)
    return true;
  if (key._paxTypeCode < _paxTypeCode)
    return false;
  if (_currencyCode < key._currencyCode)
    return true;
  if (key._currencyCode < _currencyCode)
    return false;

  return false;
}
