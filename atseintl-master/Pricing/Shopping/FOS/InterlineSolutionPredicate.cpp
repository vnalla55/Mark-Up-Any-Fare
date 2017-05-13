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
#include "Pricing/Shopping/FOS/InterlineSolutionPredicate.h"

#include "Pricing/Shopping/FOS/FosCommonUtil.h"

namespace tse
{
namespace fos
{

bool
InterlineSolutionPredicate::
operator()(const SopCombination& sopIds)
{
  return FosCommonUtil::detectCarrier(_trx, sopIds) == FosCommonUtil::INTERLINE_CARRIER;
}

} // fos
} // tse
