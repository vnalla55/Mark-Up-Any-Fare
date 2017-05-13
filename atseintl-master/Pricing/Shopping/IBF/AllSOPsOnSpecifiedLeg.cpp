//-------------------------------------------------------------------
//
//  Authors:     Grzegorz Szczurek
//
//  Copyright Sabre 2015
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

#include "Pricing/Shopping/IBF/AllSOPsOnSpecifiedLeg.h"

#include <sstream>

namespace tse
{

bool
AllSOPsOnSpecifiedLeg::isSatisfied() const
{
  TSE_ASSERT(_legIdToTrack != -1);
  return _tracker.getNbrOfUnusedSopsOnLeg(_legIdToTrack) == 0;
}

std::string
AllSOPsOnSpecifiedLeg::toString() const
{
  TSE_ASSERT(_legIdToTrack != -1);
  std::ostringstream out;
  out << "All SOPs on specified leg (" <<
      (_tracker.getNbrOfSops() - _tracker.getNbrOfUnusedSopsOnLeg(_legIdToTrack))
     << "/" << _tracker.getNbrOfSops() << " SOPs represented)";
  return out.str();
}

} /* namespace tse */
