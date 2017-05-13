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

#include "AtpcoTaxes/DataModel/Common/CompactOptional.h"
#include <iostream>

namespace tax
{

template <typename T>
std::ostream& operator<<(std::ostream& o, const CompactOptional<T>& val)
{
  if (o.good())
  {
    if (val.has_value())
      o << val.value();
    else
      o << "-";
  }
  return o;
}

} // namespace tax

