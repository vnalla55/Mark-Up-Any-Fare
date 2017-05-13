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

#include "Common/ShoppingUtil.h"
#include "Pricing/Shopping/FOS/FosFilter.h"

namespace tse
{
class ShoppingTrx;

namespace fos
{

class CustomFilter : public FosFilter
{
public:
  explicit CustomFilter(const ShoppingTrx& trx) : _trx(trx) {}

  virtual FilterType getType() const override { return FILTER_CUSTOM; }

  virtual ValidatorBitMask getAffectedValidators() const override
  {
    return validatorBitMask(VALIDATOR_CUSTOM);
  }

  virtual bool isApplicableSolution(const SopCombination& sopCombination) const override
  {
    return ShoppingUtil::isCustomSolution(_trx, sopCombination);
  }

private:
  const ShoppingTrx& _trx;
};

} // fos
} // tse
