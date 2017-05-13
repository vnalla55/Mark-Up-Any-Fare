#pragma once
// ----------------------------------------------------------------
//
//   Copyright Sabre 2016
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

#include "boost/optional/optional.hpp"

#include <string>

#include "Common/Money.h"

namespace tse
{

struct AncillaryPriceModifier
{
  enum class Type { DISCOUNT, RISE };

  boost::optional<std::string> _identifier;
  unsigned int _quantity = 1;
  boost::optional<Type> _type;
  boost::optional<unsigned int> _percentage;
  boost::optional<Money> _money;
};

}
