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
#include "Pricing/Shopping/FOS/LongConxValidator.h"

#include "Common/ShoppingUtil.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/FOS/FosStatistic.h"

namespace tse
{
namespace fos
{

BaseValidator::ValidationResult
LongConxValidator::validate(const SopIdVec& combination) const
{
  uint32_t count = getStats().getCounter(getType());
  uint32_t limit = getStats().getCounterLimit(getType());

  if (count < limit && ShoppingUtil::isLongConnection(getTrx(), combination))
    return VALID;
  return INVALID;
}

bool
LongConxValidator::isThrowAway(const SopIdVec& combination, ValidatorBitMask validBitMask) const
{
  ValidatorBitMask otherValidatorsBitMask = validBitMask & (~validatorBitMask(getType()));

  uint32_t count = getStats().getCounter(getType());
  uint32_t limit = getStats().getCounterLimit(getType());

  return (!otherValidatorsBitMask || count >= limit) &&
         ShoppingUtil::isLongConnection(getTrx(), combination);
}

} // fos
} // tse
