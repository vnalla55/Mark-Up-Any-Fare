//----------------------------------------------------------------------------
////
////  Copyright Sabre 2005
////
////      The copyright to the computer program(s) herein
////      is the property of Sabre.
////      The program(s) may be used and/or copied only with
////      the written permission of Sabre or in accordance
////      with the terms and conditions stipulated in the
////      agreement/contract under which the program(s)
////      have been supplied.
////
////----------------------------------------------------------------------------
#pragma once
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"

namespace tse
{

/**
 *   @class PaxTypeFareResultKey
 *
 *   Description:
 *   PaxTypeFareResultKey is a key for finding record2 validation result
 *     indicator1,
 *     directionality,
 *     paxType,
 *     psgAge
 *
 */

class PaxTypeFareResultKey
{
  typedef Code<6> CombinedPaxTypeCode;

public:
  PaxTypeFareResultKey() : _indicator1(' '), _directionality(BOTH), _psgAge(0) {}

  PaxTypeFareResultKey(Indicator indicator1,
                       Directionality directionality,
                       const PaxTypeCode& paxType,
                       uint16_t psgAge)
    : _indicator1(indicator1), _directionality(directionality), _paxType(paxType), _psgAge(psgAge)
  {
  }

  bool operator<(const PaxTypeFareResultKey& key) const
  {
    if (_indicator1 < key._indicator1)
      return true;
    if (_indicator1 > key._indicator1)
      return false;
    if (_directionality < key._directionality)
      return true;
    if (_directionality > key._directionality)
      return false;
    if (_paxType < key._paxType)
      return true;
    if (_paxType > key._paxType)
      return false;
    if (UNLIKELY(_psgAge < key._psgAge))
      return true;
    if (UNLIKELY(_psgAge > key._psgAge))
      return false;

    return false;
  }

  bool operator==(const PaxTypeFareResultKey& key) const
  {
    return _indicator1 == key._indicator1 && _directionality == key._directionality &&
           _paxType == key._paxType && _psgAge == key._psgAge;
  }

  struct Hash
  {
    size_t operator()(const PaxTypeFareResultKey& key) const
    {
      size_t hash(0);
      boost::hash_combine(hash, key.indicator1());
      boost::hash_combine(hash, key.directionality());
      key.paxType().hash_combine(hash);
      boost::hash_combine(hash, key.psgAge());

      return hash;
    }
  };

  Indicator indicator1() const { return _indicator1; }
  Directionality directionality() const { return _directionality; }
  const CombinedPaxTypeCode& paxType() const { return _paxType; }
  uint16_t psgAge() const { return _psgAge; }

  Indicator& indicator1() { return _indicator1; }
  Directionality& directionality() { return _directionality; }
  CombinedPaxTypeCode& paxType() { return _paxType; }
  uint16_t& psgAge() { return _psgAge; }

private:
  Indicator _indicator1;
  Directionality _directionality;
  CombinedPaxTypeCode _paxType;
  uint16_t _psgAge;
};
}

