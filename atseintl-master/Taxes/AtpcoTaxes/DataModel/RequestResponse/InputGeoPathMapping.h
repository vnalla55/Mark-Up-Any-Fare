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

#include "DataModel/RequestResponse/InputMapping.h"

#include <boost/ptr_container/ptr_vector.hpp>

namespace tax
{

struct InputGeoPathMapping
{
  bool operator==(const InputGeoPathMapping& rhs) const
  {
    if (_mappings.size() != rhs._mappings.size())
    {
      return false;
    }

    for (std::size_t i = 0; i < _mappings.size(); ++i)
    {
      if (!(_mappings[i] == rhs._mappings[i]))
      {
        return false;
      }
    }

    return true;
  }

  type::Index _id;
  boost::ptr_vector<InputMapping> _mappings;
};

} // namespace tax
