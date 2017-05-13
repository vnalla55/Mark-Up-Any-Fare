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
#include "Pricing/Shopping/FOS/ForcedConnectionPredicate.h"

#include "Common/ShoppingUtil.h"
#include "Pricing/Shopping/Utils/ShoppingUtils.h"

namespace tse
{
namespace fos
{

bool
ForcedConnectionPredicate::
operator()(const SopCombination& sopIds)
{
  return ShoppingUtil::checkForcedConnection(sopIds, _trx);
}

} // fos
} // tse
