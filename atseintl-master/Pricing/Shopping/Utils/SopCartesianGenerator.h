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

#pragma once

#include "Pricing/Shopping/Utils/SopCombinationsGenerator.h"
#include "Util/CartesianProduct.h"

namespace tse
{

namespace utils
{


// Assume s1, s2, ..., sn are the numbers of
// SOPs on legs 1, 2, ..., n.
// next() runs in O(n) building a n-tuple.
//
// Generating all possible combinations
// (calling this function multiple times)
// takes O(s1 * s2 * ... * sn * n).
class SopCartesianGenerator: public BaseSopCombinationsGenerator
{
public:
  SopCartesianGenerator(ISopBank* sopBank = new SopBank()):
    BaseSopCombinationsGenerator(sopBank){}

  void initialize() override;

  SopCombination nextElement() override;

private:
  // A generic cartesian product generator
  CartesianProduct<SopCombination> _cartProd;
};



} // namespace utils

} // namespace tse


