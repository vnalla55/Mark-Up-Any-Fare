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
#include "Pricing/Shopping/FOS/OnlineValidator.h"

#include "Common/ShoppingUtil.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/FOS/FosCommonUtil.h"
#include "Pricing/Shopping/FOS/FosStatistic.h"

namespace tse
{
namespace fos
{

BaseValidator::ValidationResult
OnlineValidator::validate(const SopIdVec& combination) const
{
  uint32_t count = getStats().getCounter(getType());
  uint32_t limit = getStats().getCounterLimit(getType());

  if (count >= limit)
    return INVALID;
  if (ShoppingUtil::isDirectFlightSolution(getTrx(), combination))
    return INVALID;
  CarrierCode cxr = FosCommonUtil::detectCarrier(getTrx(), combination);
  if (cxr == FosCommonUtil::INTERLINE_CARRIER)
    return INVALID;

  FosStatistic::CarrierCounter& cxrCounter = getStats().getCarrierCounter(cxr);
  if (cxrCounter.value < cxrCounter.limit)
    return VALID;
  return DEFERRED;
}

bool
OnlineValidator::isThrowAway(const SopIdVec& combination, ValidatorBitMask validBitMask) const
{
  return false;
}

} // fos
} // tse
