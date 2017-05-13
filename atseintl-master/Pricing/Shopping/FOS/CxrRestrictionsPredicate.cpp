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
#include "Pricing/Shopping/FOS/CxrRestrictionsPredicate.h"

#include "Common/ShoppingUtil.h"
#include "Pricing/Shopping/FOS/FosCommonUtil.h"

namespace tse
{
namespace fos
{

bool
CxrRestrictionsPredicate::
operator()(const SopCombination& sopIds)
{
  CarrierCode cxr = FosCommonUtil::detectCarrier(_trx, sopIds);
  if (cxr != FosCommonUtil::INTERLINE_CARRIER)
    return true;

  for (SopIdVec::size_type legIdx = 0; legIdx != sopIds.size(); ++legIdx)
  {
    const ShoppingTrx::SchedulingOption& sop = _trx.legs()[legIdx].sop()[sopIds[legIdx]];
    if (sop.combineSameCxr())
      return false;
  }
  return true;
}

} // fos
} // tse
