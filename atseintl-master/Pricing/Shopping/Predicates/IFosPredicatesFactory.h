
//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2013
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

#include "Pricing/Shopping/FiltersAndPipes/INamedPredicate.h"
#include "Pricing/Shopping/Utils/ShoppingUtilTypes.h"

#include <string>

namespace tse
{

namespace utils
{

enum FOS_PREDICATE_TYPE
{
  CABIN_CLASS_VALIDITY = 0,
  MINIMUM_CONNECT_TIME,
  POSITIVE_CONNECT_TIME,
  INTERLINE_TICKETING_AGREEMENT
};

class IFosPredicatesFactory
{
public:
  virtual INamedPredicate<SopCombination>*
  createFosPredicate(FOS_PREDICATE_TYPE type, const std::string& name = "") = 0;
  virtual ~IFosPredicatesFactory() {}
};

} // namespace utils

} // namespace tse

