//-------------------------------------------------------------------
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
#include "DataModel/ItinHelperStructs.h"

namespace tse
{
class Itin;

bool
ItinBoolMap::getValForKey(const Itin* key) const
{
  if (key)
  {
    ItinBoolMap::const_iterator it;

    if (end() != (it = find(key)))
      return it->second;
  }

  return false;
}

} // tse
