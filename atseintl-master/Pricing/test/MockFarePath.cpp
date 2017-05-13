//-------------------------------------------------------------------
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "Pricing/test/MockFarePath.h"
#include "DataModel/PricingUnit.h"

using namespace tse;

MockFarePath::MockFarePath() {}

MockFarePath::~MockFarePath() {}

bool
MockFarePath::addPU(PricingUnit* pu)
{
  if (!pu)
  {
    return false;
  }

  pu->puType() = PricingUnit::Type::ONEWAY;
  _pricingUnit.push_back(pu);

  return true;
}
