// -------------------------------------------------------------------
//
//
//  Copyright (C) Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
// -------------------------------------------------------------------
#pragma once

#include "Pricing/Shopping/FOS/FosMatrixInserter.h"

#include <vector>

namespace tse
{

class Diag910Collector;

namespace fos
{

class DirectFamilyGroupingInserter : public FosMatrixInserter
{
public:
  DirectFamilyGroupingInserter(ShoppingTrx& trx, Diag910Collector* dc910);

  virtual void addCombination(ValidatorBitMask validBitMask, const SopIdVec& combination) override;

private:
  std::vector<SopIdVec> _baseSolutions;
  bool _familyGroupingEnabled;
};

} // fos
} // tse

