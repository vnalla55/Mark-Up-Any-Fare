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
#include <sstream>
#include "Common/TaxableUnitTagSet.h"

namespace tax
{

std::string noMatchingTutMessage(TaxableUnitTagSet const& tutSet)
{
  std::ostringstream ans;
  ans << "NO TAXES FOR MARKED TAXABLE UNIT TAGS:";

  for (int i = 0; i < 11; ++i)
  {
    if (tutSet.hasBit(i))
    {
      ans << " ";
      ans << i;
    }
  }

  return ans.str();
}

} // namespace tax
