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
#include "Pricing/Shopping/FOS/CxrOverridePredicate.h"

#include "Common/ShoppingUtil.h"

namespace tse
{
namespace fos
{

bool
CxrOverridePredicate::
operator()(const SopCombination& sopIds)
{
  return _trx.getRequest()->cxrOverride() == BLANK_CODE ||
         ShoppingUtil::checkOverridedSegment(sopIds, _trx);
}

} // fos
} // tse
