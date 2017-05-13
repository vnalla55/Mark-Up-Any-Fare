//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2014
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

#include "Pricing/Shopping/Utils/SopCartesianGenerator.h"

#include "Common/Logger.h"

#include <sstream>

namespace tse
{

namespace utils
{

namespace
{
Logger
logger("atseintl.Pricing.ShoppingUtils.SopCartesianGenerator");
}

void
SopCartesianGenerator::initialize()
{
  if (IS_DEBUG_ENABLED(logger))
  {
    std::ostringstream out;
    out << "Feeding generator with SOPs per leg:\n" << _userInputSops->toString();
    LOG4CXX_DEBUG(logger, out.str());
  }

  for (unsigned int i = 0; i < _userInputSops->getNumberOfLegs(); ++i)
  {
    _cartProd.addSet(_userInputSops->getSopsOnLeg(i));
  }
}


SopCombination SopCartesianGenerator::nextElement()
{
  // _cartProd will return empty set forever
  // if cartesian combinations exhausted
  // - just need to forward
  const CartesianProduct<SopCombination>::ProductType combination
      = _cartProd.getNext();
  return SopCombination(combination.begin(), combination.end());
}



} // namespace utils

} // namespace tse
