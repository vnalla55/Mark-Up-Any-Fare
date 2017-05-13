//----------------------------------------------------------------------------
//
//  Copyright Sabre 2005
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------
#pragma once
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"

namespace tse
{

/**
*   @class FareMarketCurrencyKey
*
*   Description:
*   FareMarketCurrencyKey is a key for a set of FareMarket currencies by
*   passenger type, direction and currencies.
*
*/

class FareMarketCurrencyKey
{
public:
  FareMarketCurrencyKey(const PaxTypeCode& paxType, const int& dir, const CurrencyCode& currency);

  FareMarketCurrencyKey(const PaxTypeCode& paxType, const int& dir);

  FareMarketCurrencyKey(const FareMarketCurrencyKey& key);

  const PaxTypeCode& paxTypeCode() const { return _paxTypeCode; }
  PaxTypeCode& paxTypeCode() { return _paxTypeCode; }

  const CurrencyCode& currencyCode() const { return _currencyCode; }
  CurrencyCode& currencyCode() { return _currencyCode; }

  const int& directionality() const { return _directionality; }
  int& directionality() { return _directionality; }

  bool operator<(const FareMarketCurrencyKey& key) const;

  struct CompareKeys
  {
    bool operator()(const FareMarketCurrencyKey& key1, const FareMarketCurrencyKey& key2);
  };

private:
  PaxTypeCode _paxTypeCode;
  int _directionality;
  CurrencyCode _currencyCode;
};
}

