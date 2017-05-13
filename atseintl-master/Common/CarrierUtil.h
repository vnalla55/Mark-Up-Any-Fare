//-------------------------------------------------------------------
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
//-------------------------------------------------------------------

#pragma once

#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"

namespace tse
{
class PricingTrx;

namespace CarrierUtil
{
bool
carrierAllianceMatch(const CarrierCode& cxr, const CarrierCode& restriction, const PricingTrx& trx);

bool
carrierExactOrAllianceMatch(const CarrierCode& cxr,
                            const CarrierCode& restriction,
                            const PricingTrx& trx);

inline bool
isAllianceCode(const CarrierCode& cxr)
{
  return cxr == ONE_WORLD_ALLIANCE || cxr == SKY_TEAM_ALLIANCE || cxr == STAR_ALLIANCE;
}
}
}

