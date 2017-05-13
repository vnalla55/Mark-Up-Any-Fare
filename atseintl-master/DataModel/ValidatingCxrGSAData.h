#pragma once

#include "Common/Global.h"
#include "Common/Hasher.h"
#include "Common/ValidatingCxrConst.h"

#include <tr1/unordered_map>

namespace tse
{

// Key is potential validating carrier (can be marketing, swapped or neutral.
// Value is the related data with this carrier
typedef std::map< CarrierCode, vcx::ValidatingCxrData > ValidatingCxrDataMap;

// Key is marketing carrier, value is the gsa swaps of this carrier
typedef std::map< CarrierCode, std::set<CarrierCode> > GSASwapMap;

class ValidatingCxrGSAData
{
public:

  struct hash_func
  {
    size_t operator()(const std::string& key) const
    {
      tse::Hasher hasher(tse::Global::hasherMethod());
      hasher << key;
      return hasher.hash();
    }
  };

  ValidatingCxrGSAData() : _isNeutralValCxr(false) {}

  ValidatingCxrDataMap& validatingCarriersData() { return _validatingCarriersData; }
  const ValidatingCxrDataMap& validatingCarriersData() const { return _validatingCarriersData; }

  GSASwapMap& gsaSwapMap() { return _gsaSwapMap; }
  const GSASwapMap& gsaSwapMap() const { return _gsaSwapMap; }

  bool& isNeutralValCxr() { return _isNeutralValCxr; }
  const bool isNeutralValCxr() const { return _isNeutralValCxr; }

  std::string& errorMessage() { return _errorMessage; }
  const std::string& errorMessage() const { return _errorMessage; }

  bool getSwapCarriers(const CarrierCode& carrier, std::set<CarrierCode>& result) const
  {
    GSASwapMap::const_iterator i = _gsaSwapMap.find(carrier);
    if (i != _gsaSwapMap.end())
    {
      result = i->second;
      return true;
    }
    return false;
  }

  //@todo Change size() to numberOfValidatingCxrs() or validatingCxrCount()
  size_t size() const { return _validatingCarriersData.size(); }

  bool hasCarrier(const CarrierCode& carrier) const
    { return _validatingCarriersData.count(carrier); }

private:

  bool _isNeutralValCxr;
  std::string _errorMessage;
  ValidatingCxrDataMap _validatingCarriersData;
  GSASwapMap _gsaSwapMap;
};

// Key is the combination of marketing and operating carriers in the form:
// M1M2M3|O1O2O3, both sorted alphabetically.
typedef std::tr1::unordered_map<std::string,
                                ValidatingCxrGSAData*,
                                ValidatingCxrGSAData::hash_func> ValidatingCxrGSADataHashMap;

typedef std::map<SettlementPlanType, const ValidatingCxrGSAData*> SpValidatingCxrGSADataMap;
typedef std::tr1::unordered_map<std::string,
        SpValidatingCxrGSADataMap*,
        ValidatingCxrGSAData::hash_func> HashSpValidatingCxrGSADataMap;

} // tse

