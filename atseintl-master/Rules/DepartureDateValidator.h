//-------------------------------------------------------------------
//
//  Copyright Sabre 2010
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/TsePrimitiveTypes.h"

#include <vector>

namespace tse
{

class DateTime;
class ProcessTagPermutation;
class RexPricingTrx;
class DateTime;
class PricingUnit;
class FareMarket;
class Itin;
class TravelSeg;

class DepartureDateValidator
{
  friend class DepartureDateValidatorTest;

public:
  DepartureDateValidator() = default;

  explicit DepartureDateValidator(const RexPricingTrx& trx) : _trx(&trx) {}

  void assign(const RexPricingTrx& trx)
  {
    _trx = &trx;
    _isValid = false;
    _toAdvResMeasurePoint = ' ';
  }

  bool isValid() const { return _isValid; }
  void processPermutations();
  const DateTime& getDepartureDate(const FareMarket& fm, const Itin& itin) const;
  const DateTime& getDepartureDate(const PricingUnit& pu, const Itin& itin) const;

private:
  using Permutations = std::vector<ProcessTagPermutation*>;

  bool hasSameMeasurePoint(const Permutations& permutations, Indicator& measurePoint) const;
  const DateTime& getDepartureDate(const TravelSeg& ts) const;

  const RexPricingTrx* _trx = nullptr;
  bool _isValid = false;
  Indicator _toAdvResMeasurePoint = ' ';
};

} // tse

