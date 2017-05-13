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

#include "Common/TseStlTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Pricing/Shopping/FOS/FosTypes.h"

namespace tse
{

class Diag910Collector;
class ShoppingTrx;

namespace fos
{

class FosMatrixInserter
{
public:
  FosMatrixInserter(ShoppingTrx& trx, Diag910Collector* dc910) : _trx(trx), _dc910(dc910) {}

  virtual ~FosMatrixInserter() {}

  ShoppingTrx& getTrx() const { return _trx; }

  virtual void addCombination(ValidatorBitMask validBitMask, const SopIdVec& combination);

protected:
  ShoppingTrx& _trx;
  Diag910Collector* _dc910;
};

} // fos
} // tse
