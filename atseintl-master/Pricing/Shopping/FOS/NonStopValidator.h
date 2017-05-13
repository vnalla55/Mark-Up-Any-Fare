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

#include "Pricing/Shopping/FOS/BaseValidator.h"

namespace tse
{
namespace fos
{

class NonStopValidator : public BaseValidator
{
public:
  NonStopValidator(ShoppingTrx& trx, FosBaseGenerator& generator, FosStatistic& stats)
    : BaseValidator(trx, generator, stats), _enableDeferredProcessing(false)
  {
  }

  virtual ValidatorType getType() const override { return VALIDATOR_NONSTOP; }
  virtual ValidationResult validate(const SopIdVec& combination) const override;
  virtual bool isThrowAway(const SopIdVec& combination, ValidatorBitMask validBitMask) const override;

  void setDeferredProcessing(bool isEnabled) { _enableDeferredProcessing = isEnabled; }

private:
  bool _enableDeferredProcessing;
};

} // fos
} // tse
