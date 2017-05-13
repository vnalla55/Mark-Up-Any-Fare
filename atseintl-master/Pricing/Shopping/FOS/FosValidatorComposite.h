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

#include "Common/TseStlTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Pricing/Shopping/FOS/FosTypes.h"

#include <vector>

namespace tse
{
namespace fos
{

class BaseValidator;

class FosValidatorComposite
{
public:
  void addValidator(BaseValidator& validator) { _validators.push_back(&validator); }

  void validate(const SopIdVec& combination,
                ValidatorBitMask& validBitMask,
                ValidatorBitMask& deferredBitMask,
                ValidatorBitMask& invalidSopDetailsBitMask) const;

  bool isThrowAway(const SopIdVec& combination, ValidatorBitMask validBitMask) const;

private:
  std::vector<BaseValidator*> _validators;
};

} // fos
} // tse
