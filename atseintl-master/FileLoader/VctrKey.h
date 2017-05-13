#pragma once

namespace tse
{
class VctrKey
{
  friend std::ostream& operator<<(std::ostream& os, const VctrKey& key);
  friend bool operator<(const VctrKey& first, const VctrKey& second);

public:
  VctrKey(const FareInfo& fareInfo)
    : _vendor(fareInfo._vendor),
      _carrier(fareInfo._carrier),
      _fareTariff(fareInfo._fareTariff),
      _ruleNumber(fareInfo._ruleNumber)
  {
  }

private:
  VendorCode _vendor;
  CarrierCode _carrier;
  TariffNumber _fareTariff;
  RuleNumber _ruleNumber;
};

std::ostream& operator<<(std::ostream& os, const VctrKey& key)
{
  os << key._vendor << ' ' << key._carrier << ' ' << key._fareTariff << ' ' << key._ruleNumber;
  return os;
}

bool operator<(const VctrKey& first, const VctrKey& second)
{
  if (first._vendor < second._vendor)
    return true;
  if (first._vendor > second._vendor)
    return false;

  if (first._carrier < second._carrier)
    return true;
  if (first._carrier > second._carrier)
    return false;

  if (first._fareTariff < second._fareTariff)
    return true;
  if (first._fareTariff > second._fareTariff)
    return false;

  if (first._ruleNumber < second._ruleNumber)
    return true;
  if (first._ruleNumber > second._ruleNumber)
    return false;

  return false;
}
typedef std::set<VctrKey> VctrSet;
}
