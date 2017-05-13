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
#include "Pricing/Shopping/FOS/FosValidatorComposite.h"

#include "Pricing/Shopping/FOS/BaseValidator.h"

namespace tse
{
namespace fos
{

void
FosValidatorComposite::validate(const SopIdVec& combination,
                                ValidatorBitMask& validBitMask,
                                ValidatorBitMask& deferredBitMask,
                                ValidatorBitMask& invalidSopDetailsBitMask) const
{
  validBitMask = deferredBitMask = invalidSopDetailsBitMask = 0;

  for (BaseValidator* validator : _validators)
  {
    BaseValidator::ValidationResult result = validator->validate(combination);
    if (result == BaseValidator::VALID)
      validBitMask |= validatorBitMask(validator->getType());
    else if (result == BaseValidator::DEFERRED)
      deferredBitMask |= validatorBitMask(validator->getType());
    else if (result == BaseValidator::INVALID_SOP_DETAILS)
      invalidSopDetailsBitMask |= validatorBitMask(validator->getType());
  }
}

bool
FosValidatorComposite::isThrowAway(const SopIdVec& combination, ValidatorBitMask validBitMask) const
{
  for (BaseValidator* validator : _validators)
  {
    if (validator->isThrowAway(combination, validBitMask))
      return true;
  }

  return false;
}

} // fos
} // tse
