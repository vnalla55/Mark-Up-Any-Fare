//----------------------------------------------------------------------------
//
// Copyright Sabre 2015
//
//     The copyright to the computer program(s) herein
//     is the property of Sabre.
//     The program(s) may be used and/or copied only with
//     the written permission of Sabre or in accordance
//     with the terms and conditions stipulated in the
//     agreement/contract under which the program(s)
//     have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "DataModel/PaxType.h"

namespace tse
{
class Diag852Collector;
class PricingTrx;
struct FreqFlyerStatusData;

namespace freqflyerutils
{
uint16_t
determineFreqFlyerTierLevel(Diag852Collector* dc,
                            std::vector<PaxType::FreqFlyerTierWithCarrier*>& freqFlyerData,
                            CarrierCode carrier,
                            PricingTrx* trx);
bool
checkReadingTableA03(const std::vector<PaxType::FreqFlyerTierWithCarrier*>& freqFlyerData,
                     const CarrierCode carrier,
                     Diag852Collector* dc);

void
printDeterminedFFStatus(Diag852Collector* dc, const uint16_t frequentFlyerTierLevel);

bool
checkCarrierPresenceInTableA03(const std::vector<FreqFlyerStatusData>& carrierData);
}
}
