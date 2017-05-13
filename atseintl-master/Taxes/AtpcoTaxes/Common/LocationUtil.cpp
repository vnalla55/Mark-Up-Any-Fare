// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

#include "Common/LocationUtil.h"
#include "Rules/TimeStopoverChecker.h"

namespace tax
{

namespace LocationUtil
{

const type::Nation RUSSIA_EUROPE{"RU"};
const type::Nation RUSSIA_ASIA{"XU"};

bool isDomestic(const type::Nation& nation1, const type::Nation& nation2)
{
  return (nation1 == nation2) || isBothRussia(nation1, nation2);
}

bool isInternational(const type::Nation& nation1, const type::Nation& nation2)
{
  return !isDomestic(nation1, nation2);
}

bool isBothRussia(const type::Nation& nation1, const type::Nation& nation2)
{
  return (nation1 == RUSSIA_ASIA && nation2 == RUSSIA_EUROPE) ||
         (nation1 == RUSSIA_EUROPE && nation2 == RUSSIA_ASIA);
}

bool doTaxLoc1AndLoc2NationMatch(LocZone loc1, LocZone loc2)
{
  return loc1.type() == type::LocType::Nation &&
         loc2.type() == type::LocType::Nation &&
         loc1.code() == loc2.code();
}

}

} // end of tax namespace
