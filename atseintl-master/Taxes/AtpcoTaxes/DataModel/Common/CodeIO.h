// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#pragma once

#include <iostream>
#include "AtpcoTaxes/DataModel/Common/Code.h"

namespace tax
{

template <typename tag, int L, int H>
std::ostream& operator<<(std::ostream& o, const Code<tag, L, H>& code)
{
  if (o.good())
    o << code.asString();
  return o;
}

} // namespace tax

