//-------------------------------------------------------------------
//
//  File:        CxrPrecalculatedTaxes.h
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

#pragma once

#include "Common/TseCodeTypes.h"
#include "DataModel/PrecalculatedTaxes.h"

namespace tse
{


class CxrPrecalculatedTaxes
{
public:

  typedef FlatMap<CarrierCode, PrecalculatedTaxes>::const_iterator const_iterator;

  const_iterator begin() const { return _cxrPrecalculatedTaxes.begin(); }
  const_iterator end() const { return _cxrPrecalculatedTaxes.end(); }

  void processLowerBoundAmounts();

  const PrecalculatedTaxes& operator[](CarrierCode cxr) const
  { return _cxrPrecalculatedTaxes.at(cxr); }

  PrecalculatedTaxes& operator[](CarrierCode cxr)
  { return _cxrPrecalculatedTaxes[cxr]; }

  bool operator==(const CxrPrecalculatedTaxes& other) const
  { return _cxrPrecalculatedTaxes == other._cxrPrecalculatedTaxes; }

  MoneyAmount getLowerBoundTotalTaxAmount(const PaxTypeFare& fare) const;
  MoneyAmount getTotalTaxAmountForCarrier(const PaxTypeFare& fare, const CarrierCode cxr) const;

  void copyAmountsIfPresent(const CxrPrecalculatedTaxes& other,
                            const PaxTypeFare& otherFare,
                            const PaxTypeFare& localFare);

  bool empty() const { return _cxrPrecalculatedTaxes.empty(); }
  void clear() { _cxrPrecalculatedTaxes.clear(); }

  bool isProcessed() const
  { return (!_cxrPrecalculatedTaxes.empty() &&
            _cxrPrecalculatedTaxes.begin()->second.isProcessed(PrecalculatedTaxesAmount::CAT12)); }

private:

  FlatMap<CarrierCode, PrecalculatedTaxes> _cxrPrecalculatedTaxes;
  FlatMap<const PaxTypeFare*, MoneyAmount> _lowerBoundTaxPerFare;
  MoneyAmount _defaultLowerBoundTotalTax = 0.0;
};


} // namespace
