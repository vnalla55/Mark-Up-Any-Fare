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

class OutputTaxDetailsRef
{
public:
  explicit OutputTaxDetailsRef(const type::Index& id) : _id(id) {}

  bool operator==(const OutputTaxDetailsRef& other) const { return _id == other._id; }

  const type::Index& id() const
  {
    return _id;
  };
  type::Index& id()
  {
    return _id;
  };

private:
  type::Index _id;
};
}
