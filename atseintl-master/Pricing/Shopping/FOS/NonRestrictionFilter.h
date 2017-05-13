// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------
#pragma once

#include "Pricing/Shopping/FOS/FosFilter.h"

namespace tse
{
class ShoppingTrx;

namespace fos
{
class FosTaskScope;
class FosStatistic;

class NonRestrictionFilter : public FosFilter
{
public:
  NonRestrictionFilter(const ShoppingTrx& trx, const FosTaskScope& task);

  virtual FilterType getType() const override { return FILTER_NONRESTRICTION; }

  virtual ValidatorBitMask getAffectedValidators() const override
  {
    return _canProduceNonStops ? ~ValidatorBitMask(0) : ~validatorBitMask(VALIDATOR_NONSTOP);
  }

  virtual bool isApplicableSolution(const SopCombination& sopCombination) const override;
  virtual bool isFilterToPop(const FosStatistic& stats) const override;

  void setNumTSRestrictions(const size_t totalNumTS, const size_t numTSPerLeg);

private:
  const ShoppingTrx& _trx;

  size_t _numTSPerLeg;
  size_t _totalNumTS;
  bool _checkConnectingCities;
  bool _canProduceNonStops;
  const bool _isNoPricedSolutions;
  const bool _isPqOverride;
};

} // fos
} // tse

