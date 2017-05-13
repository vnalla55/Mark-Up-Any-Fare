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
*   @class CTRWCurrencyKey
*
*   Description:
*   CTRWCurrencyKey is a key for a set of FareMarket currencies by
*   passenger type, and currencies.
*
*/

class CTRWCurrencyKey
{
public:
  CTRWCurrencyKey(const PaxTypeCode& paxType, const CurrencyCode& currency);

  const PaxTypeCode& paxTypeCode() const { return _paxTypeCode; }
  PaxTypeCode& paxTypeCode() { return _paxTypeCode; }

  const CurrencyCode& currencyCode() const { return _currencyCode; }
  CurrencyCode& currencyCode() { return _currencyCode; }

  bool operator<(const CTRWCurrencyKey& key) const;

  struct CompareKeys
  {
    bool operator()(const CTRWCurrencyKey& key1, const CTRWCurrencyKey& key2);
  };

private:
  PaxTypeCode _paxTypeCode;
  CurrencyCode _currencyCode;
};
}

