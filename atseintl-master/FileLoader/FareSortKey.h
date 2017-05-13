#pragma once

// VENDOR, CARRIER, FARETARIFF, CUR, FARECLASS, SEQNO, LINKNO, CREATEDATE

namespace tse
{
class FareSortKey
{
  friend bool operator<(const FareSortKey& first, const FareSortKey& second);

public:
  FareSortKey(const FareInfo& fareInfo)
    : _vendor(fareInfo._vendor),
      _carrier(fareInfo._carrier),
      _fareTariff(fareInfo._fareTariff),
      _currency(fareInfo._currency),
      _fareClass(fareInfo._fareClass),
      _sequenceNumber(fareInfo._sequenceNumber),
      _linkNumber(fareInfo._linkNumber),
      _createDate(fareInfo._effInterval.createDate())
  {
  }

private:
  VendorCode _vendor;
  CarrierCode _carrier;
  TariffNumber _fareTariff;
  CurrencyCode _currency;
  FareClassCode _fareClass;
  SequenceNumber _sequenceNumber;
  LinkNumber _linkNumber;
  DateTime _createDate;
};

bool operator<(const FareSortKey& first, const FareSortKey& second)
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

  if (first._currency < second._currency)
    return true;
  if (first._currency > second._currency)
    return false;

  if (first._fareClass < second._fareClass)
    return true;
  if (first._fareClass > second._fareClass)
    return false;

  if (first._sequenceNumber < second._sequenceNumber)
    return true;
  if (first._sequenceNumber > second._sequenceNumber)
    return false;

  if (first._linkNumber < second._linkNumber)
    return true;
  if (first._linkNumber > second._linkNumber)
    return false;

  if (first._createDate < second._createDate)
    return true;
  if (first._createDate > second._createDate)
    return false;

  return false;
}
}
