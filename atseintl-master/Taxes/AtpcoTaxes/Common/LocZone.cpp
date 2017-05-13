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
#include "Common/SafeEnumToString.h"
#include "Common/LocZone.h"
#include "DataModel/Common/CodeIO.h"
#include <sstream>

namespace tax
{

std::string
LocZone::toString() const
{
  std::ostringstream ans;
  ans << _type << " " << _code;
  return ans.str();
}

} // namespace tax
