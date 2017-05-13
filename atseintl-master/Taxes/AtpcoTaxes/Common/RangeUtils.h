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

#include "DataModel/Common/Types.h"

namespace tax
{

struct Range
{
  Range();
  Range(type::Index b, type::Index e);

  bool operator==(const Range& other) const;
  bool operator<=(const Range& other) const;
  Range operator&(const Range& other) const;

  type::Index begin, end;
  bool empty;
};

struct ProperRange : public Range
{
  ProperRange(type::Index b, type::Index e);
};

}
