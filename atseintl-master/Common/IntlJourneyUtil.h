//----------------------------------------------------------------------------
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

#pragma once

#include "Common/TseStlTypes.h"

#include <vector>

namespace tse
{
class AirSeg;
class Itin;
class PricingTrx;
class TravelSeg;

namespace IntlJourneyUtil
{
typedef std::vector<ClassOfServiceList> JourneyCosList;
typedef std::pair<JourneyCosList*, std::vector<TravelSeg*> > CosAndKey;
typedef std::vector<CosAndKey> Journeys;

void
constructJourneyInfo(PricingTrx& trx, Itin& itin);
void
constructJourneyInfo(PricingTrx& trx);

bool
processTsAlreadyInJourney(const Journeys& journeys, TravelSeg* ts, Itin& itin);
void
determineRemainingSegs(const CarrierCode& cxr,
                       const Journeys& journeys,
                       const Itin& itin,
                       std::vector<TravelSeg*>& segs);
bool
getInterlineKey(AirSeg* startSeg,
                const std::vector<TravelSeg*>& inputSegs,
                std::vector<TravelSeg*>& key);
}
}

