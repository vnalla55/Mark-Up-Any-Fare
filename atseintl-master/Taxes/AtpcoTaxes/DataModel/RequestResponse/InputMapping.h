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

#include <boost/ptr_container/ptr_vector.hpp>

#include "DataModel/RequestResponse/InputMap.h"

namespace tax
{

class InputMapping
{
public:
  boost::ptr_vector<InputMap>& maps() { return _maps; }

  const boost::ptr_vector<InputMap>& maps() const { return _maps; }

  bool operator==(const InputMapping& rhs) const
  {
    if (_maps.size() != rhs._maps.size())
    {
      return false;
    }

    for (std::size_t i = 0; i < _maps.size(); ++i)
    {
      if (!(_maps[i] == rhs._maps[i]))
      {
        return false;
      }
    }

    return true;
  }

  std::ostream& print(std::ostream& out, int indentLevel = 0, char indentChar = ' ') const;

private:
  boost::ptr_vector<InputMap> _maps;
};
}
