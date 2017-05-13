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
#include "Pricing/Shopping/FOS/FosMatrixInserter.h"

#include "DataModel/ShoppingTrx.h"
#include "Diagnostic/Diag910Collector.h"

namespace tse
{
namespace fos
{

void
FosMatrixInserter::addCombination(ValidatorBitMask validBitMask, const SopIdVec& combination)
{
  _trx.flightMatrix().insert(std::make_pair(combination, (GroupFarePath*)nullptr));
  if (_dc910)
    _dc910->printFos(_trx, combination, SopIdVec(), validBitMask);
}

} // fos
} // tse
