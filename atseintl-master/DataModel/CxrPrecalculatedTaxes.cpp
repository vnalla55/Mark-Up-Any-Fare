//-------------------------------------------------------------------
//
//  File:        CxrPrecalculatedTaxes.cpp
//
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

#include "DataModel/CxrPrecalculatedTaxes.h"
#include "DataModel/PaxTypeFare.h"
#include "Util/Algorithm/Container.h"

namespace tse
{

MoneyAmount CxrPrecalculatedTaxes::getLowerBoundTotalTaxAmount(const PaxTypeFare& fare) const
{
  const auto i = _lowerBoundTaxPerFare.find(&fare);
  if (i != _lowerBoundTaxPerFare.end())
    return i->second;

  return _defaultLowerBoundTotalTax;
}

MoneyAmount CxrPrecalculatedTaxes::getTotalTaxAmountForCarrier(const PaxTypeFare& fare,
                                                               const CarrierCode cxr) const
{
  if (!_cxrPrecalculatedTaxes.empty())
  {
    const auto it = _cxrPrecalculatedTaxes.find(cxr);
    if (it != _cxrPrecalculatedTaxes.end())
      return it->second.getTotalTaxAmount(fare);
  }

  return 0.0;
}

void CxrPrecalculatedTaxes::copyAmountsIfPresent(const CxrPrecalculatedTaxes& other,
                                                 const PaxTypeFare& otherFare,
                                                 const PaxTypeFare& localFare)
{
  for (auto& i : _cxrPrecalculatedTaxes)
  {
    auto iOther = other._cxrPrecalculatedTaxes.find(i.first);
    if (LIKELY(iOther != other.end()))
      i.second.copyAmountsIfPresent(iOther->second, otherFare, localFare);
  }
}

void CxrPrecalculatedTaxes::processLowerBoundAmounts()
{
  if (_cxrPrecalculatedTaxes.empty())
    return;

  bool firstCarrier = true;
  _defaultLowerBoundTotalTax = std::numeric_limits<MoneyAmount>::max();

  for (auto& kv : _cxrPrecalculatedTaxes)
  {
    const PrecalculatedTaxes& taxes = kv.second;
    const MoneyAmount cxrDefaultAmount = taxes.getDefaultTotalTaxAmount();

    if (cxrDefaultAmount < _defaultLowerBoundTotalTax)
      _defaultLowerBoundTotalTax = cxrDefaultAmount;

    for (auto& i : taxes)
    {
      const PaxTypeFare* ptf = i.first;
      const MoneyAmount taxAmount = taxes.getTotalTaxAmount(i.second);

      if (firstCarrier)
        _lowerBoundTaxPerFare[ptf] = taxAmount;
      else
      {
        const auto f = _lowerBoundTaxPerFare.find(ptf);
        if (f == _lowerBoundTaxPerFare.end())
          _lowerBoundTaxPerFare[ptf] = taxAmount;
        else if (taxAmount < f->second)
          f->second = taxAmount;
      }
    }

    firstCarrier = false;
  }

  using Map = decltype(_lowerBoundTaxPerFare);
  using Pair = Map::value_type;
  alg::erase_if(_lowerBoundTaxPerFare, [&](const Pair& kv)
  {
    if (kv.second == _defaultLowerBoundTotalTax)
    {
      // This fare amount matches the default one. Don't store it in the map.
      return true;
    }

    return false;
  });

  _lowerBoundTaxPerFare.shrink_to_fit();
}

} // namespace
