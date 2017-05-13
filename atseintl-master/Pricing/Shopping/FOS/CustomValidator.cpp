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
#include "Pricing/Shopping/FOS/CustomValidator.h"

#include "Common/ShoppingUtil.h"
#include "Pricing/Shopping/FOS/FosStatistic.h"

namespace tse
{
namespace fos
{

BaseValidator::ValidationResult
CustomValidator::validate(const SopIdVec& combination) const
{
  uint32_t count = getStats().getCounter(getType());
  uint32_t limit = getStats().getCounterLimit(getType());

  if (count < limit && ShoppingUtil::isCustomSolution(getTrx(), combination))
    return VALID;
  return INVALID;
}

bool
CustomValidator::isThrowAway(const SopIdVec& combination, ValidatorBitMask validBitMask) const
{
  uint32_t count = getStats().getCounter(getType());
  uint32_t limit = getStats().getCounterLimit(getType());

  return count >= limit && ShoppingUtil::isCustomSolution(getTrx(), combination);
}

} // fos
} // tse
