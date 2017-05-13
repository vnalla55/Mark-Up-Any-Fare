//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
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

#include "DataModel/BrandingTrx.h"

#include "Service/Service.h"

namespace tse
{

bool
BrandingTrx::process(Service& srv)
{
  return srv.process(*this);
}

std::ostream& operator<<(std::ostream& out, const BrandingResponseType& m)
{
  for (const auto& elem : m)
  {
    out << "Itin " << elem.first->itinNum() << std::endl;
    const ProgramsForBrandMap& proForB = elem.second;
    for (const auto& jt : proForB)
    {
      out << "    Brand " << jt.first << std::endl;
      const ProgramIdSet& programSet = jt.second;
      for (const auto& ps : programSet)
      {
        out << "        Program " << ps << std::endl;
      }
    }
  }
  return out;
}

} // namespace tse
