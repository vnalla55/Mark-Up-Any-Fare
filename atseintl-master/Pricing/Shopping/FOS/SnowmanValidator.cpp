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
#include "Pricing/Shopping/FOS/SnowmanValidator.h"

#include "Common/Assert.h"
#include "Common/ShoppingUtil.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/FOS/FosStatistic.h"
#include "Pricing/Shopping/FOS/SolFosGenerator.h"

namespace tse
{
namespace fos
{

BaseValidator::ValidationResult
SnowmanValidator::validate(const SopIdVec& combination) const
{
  TSE_ASSERT(combination.size() == 2);

  uint32_t count = getStats().getCounter(getType());
  uint32_t limit = getStats().getCounterLimit(getType());

  if (count >= limit)
    return INVALID;
  if (ShoppingUtil::isDirectFlightSolution(getTrx(), combination))
    return INVALID;

  const SopDetailsPtrVec obDetailsVec = getGenerator().getSopDetails(0, combination[0]);
  const SopDetailsPtrVec ibDetailsVec = getGenerator().getSopDetails(1, combination[1]);

  for (const SopDetails* obDetails : obDetailsVec)
  {
    for (const SopDetails* ibDetails : ibDetailsVec)
    {
      if (obDetails->cxrCode[0] == ibDetails->cxrCode[1] &&
          obDetails->destAirport == ibDetails->destAirport)
      {
        return VALID;
      }
    }
  }

  return INVALID_SOP_DETAILS;
}

bool
SnowmanValidator::isThrowAway(const SopIdVec& combination, ValidatorBitMask validBitMask) const
{
  return false;
}

} // fos
} // tse
