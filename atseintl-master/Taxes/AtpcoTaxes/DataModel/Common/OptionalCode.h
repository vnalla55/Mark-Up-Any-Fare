// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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

#include "DataModel/Common/Code.h"
#include "DataModel/Common/CompactOptional.h"

namespace tax
{

template <typename Tag, int L, int H>
struct CompactOptionalTraits<Code<Tag, L, H>>
{
  static
  Code<Tag, L, H> singularValue()
  {
    const char val [H + 1] = {'^'};
    return Code<Tag, L, H>(val);
  }

  static
  bool isSingularValue (const Code<Tag, L, H>& v)
  {
    return v.rawArray()[0] == '^';
  }
};

} // namespace tax

