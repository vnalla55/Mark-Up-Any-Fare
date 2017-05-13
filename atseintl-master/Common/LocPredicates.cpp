//----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "Common/LocPredicates.h"

#include "Common/LocUtil.h"

namespace tse
{

bool
ATPReservedZonesNotEqual::operator()(const Loc* o, const Loc* d) const
{
  const NationCode& orgN = o->nation();
  const NationCode& dstN = d->nation();
  return !LocUtil::areNationsInSameATPReservedZone(orgN, dstN);
}

}
