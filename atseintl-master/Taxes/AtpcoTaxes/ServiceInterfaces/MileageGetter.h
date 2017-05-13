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

#include "DataModel/Common/Types.h"

namespace tax
{

class MileageGetter
{
public:
  MileageGetter() {}
  virtual ~MileageGetter() {}

  virtual type::Miles getSingleDistance(const type::Index& from, const type::Index& to) const = 0;
  virtual type::GlobalDirection getSingleGlobalDir(const type::Index& from, const type::Index& to) const = 0;
};
}
